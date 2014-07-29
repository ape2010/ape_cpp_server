#include <boost/bind.hpp>
#include <iostream>
#include <istream>
#include <ostream>

#include "connection.h"
#include "loghelper.h"
#include "session.h"

namespace ape {
namespace net {
unsigned int CConnection::sm_mark_ = 0;
static const int MAX_PACKET_SIZE = 16 * 1024 * 1024;

CConnection::CConnection(boost::asio::io_service &io_service, ape::protocol::ParserFactory *factory, CSession *o): 
    io_service_(io_service), socket_(io_service), resolver_(io_service), conn_timer_(io_service), id_(++sm_mark_), 
    remoteport_(0), connected_(true), close_after_write_(false), session_(o)
{
    parser_ = factory->CreateParser();
}
CConnection::~CConnection() {
    if (parser_) {
        parser_->Release();
    }
}
void CConnection::AsyncConnect(const std::string &ip, unsigned int port, int timeout) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,this[%0X], id[%u], addr[%s:%u]\n", __FUNCTION__, this, id_, ip.c_str(), port);
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
        BS_XLOG(XLOG_WARNING,"CConnection::%s, this[%0X],  error:%s\n", __FUNCTION__, this, (err.message()).c_str());
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
    socket_.get_io_service().post(boost::bind(&CConnection::AsyncReadInner, shared_from_this()));
}
void CConnection::AsyncReadInner() {    
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u]\n",__FUNCTION__,this, id_);
    
    if (remoteip_.empty() || 0 == remoteport_) {
        try {
            remoteip_ = socket_.remote_endpoint().address().to_string();
            remoteport_ = socket_.remote_endpoint().port();
        } catch(boost::system::system_error & ec) {
            BS_SLOG(XLOG_WARNING,"CConnection::"<<__FUNCTION__<<", get remoteip error, code["<<ec.code()<<"], message["<<ec.what()<<"]\n");
        }
    }

    socket_.async_read_some( boost::asio::buffer(buffer_.top(), buffer_.capacity() - 1),
            ape::common::MakeAllocHandler(read_alloc_, boost::bind(&CConnection::HandleRead, shared_from_this(),
                boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)));
}
void CConnection::HandleRead(const boost::system::error_code& err, std::size_t size) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u], size[%d], buffer_.len[%d]\n",__FUNCTION__, this, id_, size, buffer_.len());
    if (err) {
        BS_XLOG(XLOG_DEBUG, "CConnection::%s, id[%u], code[%d], message[%s]\n", __FUNCTION__, id_, err.value(), err.message().c_str());
        OnPeerClose();
        return;
    }
    connected_ = true;
    *(buffer_.top() + size) = 0;
    //BS_XLOG(XLOG_TRACE,"CConnection::%s,\n%s\n",__FUNCTION__, buffer_.top());
    buffer_.inc_loc(size);
    if (buffer_.len() > MAX_PACKET_SIZE) {
        BS_XLOG(XLOG_WARNING,"CConnection::%s, id[%u], too big packet, addr[%s:%u]\n", __FUNCTION__, id_, remoteip_.c_str(),remoteport_);
        OnPeerClose();
        return;
    }
    const char *p = buffer_.base();
    while (p < buffer_.top()) {
        ape::message::SNetMessage *msg = parser_->CreateMessage();
        msg->Init(remoteip_.c_str(), remoteport_);
        const char *last = p;
        int len = buffer_.top() - p;
        p = parser_->Decode(p, len, msg);
        if (p == NULL || p > buffer_.top()) {
            BS_XLOG(XLOG_TRACE,"CConnection::%s, id[%u], illegal packet\n%s\n", __FUNCTION__, id_, last);
            delete msg;
            OnPeerClose();
            return;
        } else if (p == last) { //no incomplete packet
            BS_XLOG(XLOG_TRACE,"CConnection::%s, id[%u], incomplete packet, waiting for read\n%s\n", __FUNCTION__, id_, p);
            delete msg;
            break;
        } else { // (p <= buffer_.top()) {
            //BS_XLOG(XLOG_TRACE,"CConnection::%s::%d, \n%s\n", __FUNCTION__, __LINE__, p);
            session_->OnRead(msg);
        }
    }
    
    /** p <= buffer_.top() */
    //BS_XLOG(XLOG_TRACE,"CConnection::%s::%d, m_pHead[0X%0X],buffer_.base[0X%0X],buffer_.len[%d]\n",__FUNCTION__,__LINE__,m_pHead,buffer_.base(),buffer_.len())
    if (p < buffer_.top()) {
        //BS_XLOG(XLOG_TRACE,"CConnection::%s::%d, read packet over, memmove the packet, len[%d]\n",__FUNCTION__,__LINE__, buffer_.top() - p);
        memmove(buffer_.base(), p, buffer_.top() - p);
    }
    buffer_.reset_loc(buffer_.top() - p);
    if (buffer_.capacity() <= 1024) {
        //BS_XLOG(XLOG_TRACE,"CConnection::%s, increase 4096 bytes\n",__FUNCTION__);
        buffer_.add_capacity();
    }    
    socket_.async_read_some(boost::asio::buffer(buffer_.top(), buffer_.capacity() - 1),
        ape::common::MakeAllocHandler(read_alloc_, boost::bind(&CConnection::HandleRead,
            shared_from_this(), boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred)));  
}
void CConnection::AsyncWrite(ape::message::SNetMessage *msg, bool close) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u]\n",__FUNCTION__, this, id_);

    ape::common::CBuffer b;
    parser_->Encode(msg, &b);
    close_after_write_ = close;
    SLenMsg lenmsg;
    lenmsg.len = (int)(b.len());
    lenmsg.buf = (void*)malloc(lenmsg.len);
    memcpy(lenmsg.buf, b.base(), lenmsg.len);
    
    bool write_in_progress = !out_queue_.empty();
    out_queue_.push_back(lenmsg);
    if (!write_in_progress) {
        boost::asio::async_write(socket_, boost::asio::buffer(out_queue_.front().buf, out_queue_.front().len),
            ape::common::MakeAllocHandler(write_alloc_, boost::bind(&CConnection::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error)));
    }
}
void CConnection::HandleWrite(const boost::system::error_code& err) {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u]\n",__FUNCTION__,this,id_);
    if (err) {
        while (!out_queue_.empty()) {
            free((void *)(out_queue_.front().buf));
            out_queue_.pop_front();
        }
        BS_XLOG(XLOG_WARNING,"CConnection::%s, error[%s]\n", __FUNCTION__, err.message().c_str());
        OnPeerClose();
        return;
    }
    
    free((void *)(out_queue_.front().buf));
    out_queue_.pop_front();
    if (!out_queue_.empty()) {
        boost::asio::async_write(socket_, boost::asio::buffer(out_queue_.front().buf,out_queue_.front().len),
            ape::common::MakeAllocHandler(write_alloc_, boost::bind(&CConnection::HandleWrite, shared_from_this(),
                boost::asio::placeholders::error)));
    }
    
    if (close_after_write_ && out_queue_.empty()) {
        OnPeerClose();
    }
}    
void CConnection::OnPeerClose() {
    BS_XLOG(XLOG_DEBUG,"CConnection::%s,connection[%0x], id[%u], connected_[%d], addr[%s:%u]\n",
        __FUNCTION__, this, id_, connected_, remoteip_.c_str(), remoteport_);
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
    if(remoteip_.empty()) {
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
    if (0 == remoteport_) {
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

