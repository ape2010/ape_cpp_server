#include <boost/bind.hpp>
#include <iostream>
#include <istream>
#include <ostream>

#include "connection.h"
#include "loghelper.h"
#include "session.h"
#include "errorcode.h"

namespace ape {
namespace net {
unsigned int CConnection::sm_mark_ = 0;
static const int MAX_PACKET_SIZE = 16 * 1024 * 1024;

CConnection::CConnection(boost::asio::io_service &io_service, ape::protocol::EProtocolType pro, CSession *o):
    io_service_(io_service), socket_(io_service), resolver_(io_service), conn_timer_(io_service), id_(++sm_mark_),
    remoteport_(0), connected_(false), close_after_write_(false), session_(o), is_dealing_read_(false)
{
    parser_ = ape::protocol::ParserFactory::CreateParser(pro);
    if (parser_ == NULL) {
        BS_XLOG(XLOG_FATAL,"CConnection::%s, parser is NULL, pro[%d]\n", __FUNCTION__, pro);
        exit(-1);
    }
}
CConnection::~CConnection() {
    if (parser_) {
        parser_->Release();
    }
}
void CConnection::AsyncConnect(const std::string &ip, unsigned int port, int timeout) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,this[%0X], id[%u], addr[%s:%u]\n", __FUNCTION__, this, id_, ip.c_str(), port);
    if (connected_) {
        BS_XLOG(XLOG_DEBUG,"CConnection::%s,this[%0X], id[%u], addr[%s:%u] has been connected\n", __FUNCTION__, this, id_, ip.c_str(), port);
        ConnectFinish(0);
        return;
    }
    char szport[16]={0};
    sprintf(szport, "%d", port);
    boost::asio::ip::tcp::resolver::query query(ip, szport);
    resolver_.async_resolve(query,
        ape::common::MakeAllocHandler(conn_alloc_, boost::bind(&CConnection::HandleResolve, shared_from_this(),
              boost::asio::placeholders::error, boost::asio::placeholders::iterator)));

    conn_timer_.expires_from_now(boost::posix_time::seconds(timeout));
    conn_timer_.async_wait(ape::common::MakeAllocHandler(timer_alloc_, boost::bind(&CConnection::HandleConnectTimeout,
                    shared_from_this(), boost::asio::placeholders::error)));
}
void CConnection::HandleResolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
    if (err) {
        BS_XLOG(XLOG_WARNING,"CConnection::%s, this[%0X], id[%u], error:%s\n", __FUNCTION__, this, id_, (err.message()).c_str());
        ConnectFinish(-2);
        return;
    }
    boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,addr[%s:%d]\n", __FUNCTION__, endpoint.address().to_string().c_str(), endpoint.port());
    socket_.async_connect(endpoint, ape::common::MakeAllocHandler(conn_alloc_, boost::bind(&CConnection::HandleConnected, shared_from_this(),
                boost::asio::placeholders::error, ++endpoint_iterator)));
}
void CConnection::HandleConnected(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,this[%0X], id[%u]\n", __FUNCTION__, this, id_);
    if (!err) {
        ConnectFinish(0);
        return;
    }
    if (endpoint_iterator == boost::asio::ip::tcp::resolver::iterator()) {
        BS_XLOG(XLOG_WARNING,"CConnection::%s, this[%0X], id[%d],  error:%s\n", __FUNCTION__, this, id_, (err.message()).c_str());
        ConnectFinish(-3);
        return;
    }
    boost::system::error_code ignore_ec;
    socket_.close(ignore_ec);
    boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
    socket_.async_connect(endpoint, ape::common::MakeAllocHandler(conn_alloc_, boost::bind(&CConnection::HandleConnected, shared_from_this(),
        boost::asio::placeholders::error, ++endpoint_iterator)));
}
void CConnection::HandleConnectTimeout(const boost::system::error_code& err) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s, this[%0X], id[%u] error:%s\n", __FUNCTION__, this, id_, (err.message()).c_str());
    if(err != boost::asio::error::operation_aborted) {
        boost::system::error_code ignore_ec;
        socket_.close(ignore_ec);
        ConnectFinish(-3);
    }
}
void CConnection::ConnectFinish(int result) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s, id[%u], result[%d]\n", __FUNCTION__, id_, result);
    boost::system::error_code ignore_ec;
    conn_timer_.cancel(ignore_ec);

    if(result == 0){
        connected_ = true;
        AsyncReadInner();
    }
    if(session_ != NULL) {
        session_->ConnectResult(result);
    }
}
void CConnection::AsyncRead() {
    connected_ = true;
    socket_.get_io_service().post(boost::bind(&CConnection::AsyncReadInner, shared_from_this()));
}
void CConnection::AsyncReadInner() {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u]\n",__FUNCTION__,this, id_);
    if (!connected_) {
        BS_XLOG(XLOG_WARNING,"CConnection::%s, has not been connected, connection[%0x], id[%u]\n",__FUNCTION__,this, id_);
        return;
    }
    if (remoteip_.empty() || 0 == remoteport_) {
        try {
            remoteip_ = socket_.remote_endpoint().address().to_string();
            remoteport_ = socket_.remote_endpoint().port();
        } catch(boost::system::system_error & ec) {
            BS_SLOG(XLOG_WARNING,"CConnection::"<<__FUNCTION__<<", get remoteip error, code["<<ec.code()<<"], message["<<ec.what()<<"]\n");
        }
    }

    if (!is_dealing_read_) {
        //buffer_.reset_loc(0);
        socket_.async_read_some( boost::asio::buffer(buffer_.top(), buffer_.capacity() - 1),
                ape::common::MakeAllocHandler(read_alloc_, boost::bind(&CConnection::HandleRead, shared_from_this(),
                    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
    }
}
void CConnection::HandleRead(const boost::system::error_code& err, std::size_t size) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u], size[%d], buffer_.len[%d]\n",__FUNCTION__, this, id_, size, buffer_.len());
    if (err) {
        buffer_.reset_loc(0);
        BS_XLOG(XLOG_DEBUG, "CConnection::%s, id[%u], code[%d], message[%s]\n", __FUNCTION__, id_, err.value(), err.message().c_str());
        OnPeerClose();
        return;
    }
    connected_ = true;
    //*(buffer_.top() + size) = 0;
    //BS_XLOG(XLOG_TRACE,"CConnection::%s,\n%s\n",__FUNCTION__, buffer_.top());
    buffer_.inc_loc(size);
    //BS_XLOG(XLOG_TRACE, "CConnection::%s, body:\n%s\n", __FUNCTION__, GetBinaryDumpInfo(buffer_.base(), buffer_.len()).c_str());
    //BS_SLOG(XLOG_TRACE, "CConnection::HandleRead body:\n" << std::string(buffer_.base(), buffer_.len()) << "\n");

    if (buffer_.len() > MAX_PACKET_SIZE) {
        BS_XLOG(XLOG_WARNING,"CConnection::%s, id[%u], too big packet, addr[%s:%u]\n", __FUNCTION__, id_, remoteip_.c_str(),remoteport_);
        OnPeerClose();
        return;
    }
    const char *p = buffer_.base();
    while (p < buffer_.top()) {
        is_dealing_read_ = true;
        ape::message::SNetMessage *msg = parser_->CreateMessage();
        msg->Init(remoteip_.c_str(), remoteport_);
        const char *last = p;
        int len = buffer_.top() - p;
        p = parser_->Decode(p, len, msg);
        BS_XLOG(XLOG_TRACE,"CConnection::%s::%d, id[%u], p[0X%0X], base[0X%0X], last[0X%0X], top[0X%0X]\n", __FUNCTION__, __LINE__, id_, p, buffer_.base(), last, buffer_.top());
        if (p == NULL || p > buffer_.top()) {
            BS_XLOG(XLOG_TRACE,"CConnection::%s, id[%u], illegal packet\n%s\n", __FUNCTION__, id_, last);
            delete msg;
            OnPeerClose();
            return;
        } else if (p == last) { //incomplete packet
            BS_XLOG(XLOG_TRACE,"CConnection::%s, id[%u], incomplete packet, waiting for read\n", __FUNCTION__, id_);
            //BS_XLOG(XLOG_TRACE, "CConnection::%s::%d, body:\n%s\n", __FUNCTION__, __LINE__, GetBinaryDumpInfo(p, buffer_.top() - p).c_str());
            delete msg;
            break;
        } else { // (p <= buffer_.top()) {
            //BS_XLOG(XLOG_TRACE,"CConnection::%s::%d, \n%s\n", __FUNCTION__, __LINE__, p);
            if(session_ != NULL) {
                session_->OnRead(msg);
            } else {
                BS_XLOG(XLOG_WARNING,"CConnection::%s::%d, session is NULL, id[%u], size[%d], buffer_.len[%d]\n",
                    __FUNCTION__, __LINE__, id_, size, buffer_.len());
                delete msg;
                OnPeerClose();
                return;
            }
        }
    }
    is_dealing_read_ = false;

    /** p <= buffer_.top() */
    //BS_XLOG(XLOG_TRACE,"CConnection::%s::%d, m_pHead[0X%0X],buffer_.base[0X%0X],buffer_.len[%d]\n",__FUNCTION__,__LINE__,m_pHead,buffer_.base(),buffer_.len())
    if (p < buffer_.top()) {
        BS_XLOG(XLOG_TRACE,"CConnection::%s::%d, read packet over, memmove the next packet, len[%d]\n",__FUNCTION__,__LINE__, buffer_.top() - p);
        memmove(buffer_.base(), p, buffer_.top() - p);
    }
    buffer_.reset_loc((p < buffer_.top()) ? (buffer_.top() - p) : 0);
    if (buffer_.capacity() <= 1024) {
        //BS_XLOG(XLOG_TRACE,"CConnection::%s, increase 4096 bytes\n",__FUNCTION__);
        buffer_.add_capacity();
    }
    AsyncReadInner();
}
void CConnection::AsyncWrite(ape::message::SNetMessage *msg, bool close) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u], out_queue_.size[%u], msg:%s\n",
        __FUNCTION__, this, id_, out_queue_.size(), msg->BriefInfo().c_str());

    if (!connected_) {
        return;
    }
    parser_->Encode(msg, &writebuf_);
    BS_SLOG(XLOG_TRACE, "CConnection::AsyncWrite body:\n" << std::string(writebuf_.base(), writebuf_.len()) << "\n");
    /*
    std::string str = ape::message::GetBinaryDumpInfo(writebuf_.base(), writebuf_.len());
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u], buf:\n%s\n",__FUNCTION__, this, id_, str.c_str());
    */
    close_after_write_ = close;
    boost::asio::async_write(socket_, boost::asio::buffer(writebuf_.base(), writebuf_.len()),
            ape::common::MakeAllocHandler(write_alloc_, boost::bind(&CConnection::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error)));
/*
    SLenMsg lenmsg;
    lenmsg.len = (int)(writebuf_.len());
    lenmsg.buf = (void*)malloc(lenmsg.len);
    memcpy(lenmsg.buf, writebuf_.base(), lenmsg.len);

    bool write_in_progress = !out_queue_.empty();
    out_queue_.push_back(lenmsg);
    boost::asio::async_write(socket_, boost::asio::buffer(lenmsg.buf, lenmsg.len),
            ape::common::MakeAllocHandler(write_alloc_, boost::bind(&CConnection::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error)));
    return;*/
/*
    if (!write_in_progress) {
        boost::asio::async_write(socket_, boost::asio::buffer(out_queue_.front().buf, out_queue_.front().len),
            ape::common::MakeAllocHandler(write_alloc_, boost::bind(&CConnection::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error)));
    }*/
}
void CConnection::HandleWrite(const boost::system::error_code& err) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u], out_queue_.size[%u]\n",__FUNCTION__,this,id_, out_queue_.size());
    if (err) {
        BS_XLOG(XLOG_WARNING,"CConnection::%s, id[%u], out_queue_.size[%u], error[%s]\n", __FUNCTION__, id_, out_queue_.size(), err.message().c_str());
        /*while (!out_queue_.empty()) {
            free((void *)(out_queue_.front().buf));
            out_queue_.pop_front();
        }*/
        OnPeerClose();
        return;
    }
/*
    if (!out_queue_.empty()) {
        free((void *)(out_queue_.front().buf));
        out_queue_.pop_front();
    }*/
    /*
    if (!out_queue_.empty()) {
        boost::asio::async_write(socket_, boost::asio::buffer(out_queue_.front().buf,out_queue_.front().len),
            ape::common::MakeAllocHandler(write_alloc_, boost::bind(&CConnection::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error)));
    }*/

    if (close_after_write_ && out_queue_.empty()) {
        /*while (!out_queue_.empty()) {
            free((void *)(out_queue_.front().buf));
            out_queue_.pop_front();
        }*/
        OnPeerClose();
    }
}
void CConnection::OnPeerClose() {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u], connected_[%d], addr[%s:%u]\n",
        __FUNCTION__, this, id_, connected_, remoteip_.c_str(), remoteport_);
    is_dealing_read_ = false;
    if (connected_ && NULL != session_) {
        session_->OnPeerClose();
        session_ = NULL;
    }
    if (connected_) {
        boost::system::error_code ignore_ec;
        socket_.close(ignore_ec);
    }
    connected_ = false;
}
const std::string &CConnection::GetRemoteIp() {
    if(remoteip_.empty() && connected_) {
        try {
            remoteip_ = socket_.remote_endpoint().address().to_string();
            remoteport_ = socket_.remote_endpoint().port();
        } catch(boost::system::system_error & ec) {
            BS_SLOG(XLOG_WARNING,"CConnection::"<<__FUNCTION__<<", error, code["<<ec.code()<<"], message["<<ec.what()<<"]\n");
        }
    }

    return remoteip_;
}
unsigned int CConnection::GetRemotePort() {
    if (0 == remoteport_ && connected_) {
        try {
            remoteip_ = socket_.remote_endpoint().address().to_string();
            remoteport_ = socket_.remote_endpoint().port();
        } catch(boost::system::system_error & ec) {
            BS_SLOG(XLOG_WARNING,"CConnection::"<<__FUNCTION__<<", error, code["<<ec.code()<<"], message["<<ec.what()<<"]\n");
        }
    }

    return remoteport_;
}
void CConnection::Dump() {
    BS_XLOG(XLOG_DEBUG,"  ==============CConnection::%s,id_[%u]]=========\n",__FUNCTION__,id_);
    BS_XLOG(XLOG_DEBUG,"           remoteip_[%s], remoteport_[%u]\n", remoteip_.c_str(),remoteport_);
    BS_XLOG(XLOG_DEBUG,"  ==============CConnection::%s==End==============================\n",__FUNCTION__);
}
}
}

