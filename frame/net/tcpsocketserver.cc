#include "tcpsocketserver.h"
#include "loghelper.h"
#include "session.h"
#include "protocolhelper.h"
#include <iostream>
#include <boost/bind.hpp>

namespace ape {
namespace net{
CTcpSocketServer::CTcpSocketServer(boost::asio::io_service &io_service, CNetServiceHolder *h):
    io_service_(io_service), acceptor_(io_service), holder_(h), running_(false)
{}
int CTcpSocketServer::StartServer(const std::string &addr) {
    BS_XLOG(XLOG_DEBUG,"CTcpSocketServer::%s, addr[%s]...\n",__FUNCTION__, addr.c_str());
    char ip[32] = {0};
    unsigned int port;
    
    if (0 != ape::protocol::ParseAddr(addr, &proto_, ip, &port)) {
        BS_XLOG(XLOG_FATAL,"CTcpSocketServer::%s, bad addr[%s]...\n",__FUNCTION__, addr.c_str());
        return -1;
    }
    boost::system::error_code ec;
    acceptor_.open(boost::asio::ip::tcp::v4(),ec);
    if(ec) {
        BS_SLOG(XLOG_ERROR,"CTcpSocketServer::"<<__FUNCTION__<<",open socket error:" <<ec.message()<<"\n");
        acceptor_.close(ec);
        return -1;
    }
    acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true),ec);
    acceptor_.set_option(boost::asio::ip::tcp::no_delay(true),ec);
    acceptor_.bind(boost::asio::ip::tcp::endpoint(boost::asio::ip::address_v4::from_string(ip), port),ec);
    if(ec) {
        BS_SLOG(XLOG_FATAL,"CTcpSocketServer::"<<__FUNCTION__<<",bind socket error:" <<ec.message()<<"\n");
        acceptor_.close(ec);
        return -1;
    }
    acceptor_.listen(boost::asio::socket_base::max_connections, ec);
    if(ec) {
        BS_SLOG(XLOG_FATAL,"CTcpSocketServer::"<<__FUNCTION__<<",listen socket error:" <<ec.message()<<"\n");
        acceptor_.close(ec);
        return -3;
    }
    running_ = true;
    StartAccept();
    return 0;
}
void CTcpSocketServer::StopServer() {
    BS_XLOG(XLOG_DEBUG,"CTcpSocketServer::%s\n",__FUNCTION__);
    io_service_.post(boost::bind(&CTcpSocketServer::HandleStop, this));
}
void CTcpSocketServer::HandleStop() {
    BS_XLOG(XLOG_DEBUG,"CTcpSocketServer::%s\n",__FUNCTION__);
    boost::system::error_code ec;
    acceptor_.close(ec);
    running_ = false;
}


void CTcpSocketServer::StartAccept() {
    BS_XLOG(XLOG_DEBUG,"CTcpSocketServer::%s...\n",__FUNCTION__);
    CNetService *netservice = holder_->GetNetService();    
    CSession *session = ape::protocol::CreateSession(proto_);
    session->Init(*(netservice->GetIoService()), proto_, netservice, NULL);
    acceptor_.async_accept(session->GetConnectPrt()->Socket(),
        MakeAllocHandler(alloc_, boost::bind(&CTcpSocketServer::HandleAccept, this, session,
          boost::asio::placeholders::error)));
}

void CTcpSocketServer::HandleAccept(CSession *session, const boost::system::error_code &err) {
    BS_XLOG(XLOG_DEBUG,"CTcpSocketServer::%s\n",__FUNCTION__);
    if (!err) {
        Connection_ptr conn = session->GetConnectPrt();
        conn->Socket().get_io_service().post(boost::bind(&CSession::OnConnected, session));
        conn->AsyncRead();    
        StartAccept();
    } else if(running_) {
        BS_SLOG(XLOG_WARNING,"CTcpSocketServer::HandleAccept, error:"<< err.message() << "\n");
        acceptor_.async_accept(session->GetConnectPrt()->Socket(),
            MakeAllocHandler(alloc_, boost::bind(&CTcpSocketServer::HandleAccept, this, session,
                boost::asio::placeholders::error)));
    } else {
        BS_SLOG(XLOG_WARNING,"CTcpSocketServer::HandleAccept, stop. error:"<< err.message() << "\n");
    }
}
}
}

