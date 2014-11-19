#ifndef _APE_MONITOR_SOCKET_SERVER_H_
#define _APE_MONITOR_SOCKET_SERVER_H_

#include <boost/asio.hpp>
#include <string>
#include <map>
#include "handleralloc.h"
#include "protocol.h"
#include "netservice.h"
#include "session.h"

namespace ape {
namespace net{
class CTcpSocketServer {
 public:
    CTcpSocketServer(boost::asio::io_service &io_service, CNetServiceHolder *h);
    virtual ~CTcpSocketServer(){}
    virtual int StartServer(const std::string &addr);
    virtual void StopServer();
 private:
    void StartAccept();
    void HandleStop();
    void HandleAccept(CSession *session, const boost::system::error_code &err);
 private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    ape::common::CHandlerAllocator alloc_;
    CNetServiceHolder *holder_;
    ape::protocol::EProtocolType proto_;
    bool running_;
};
}
}
#endif

