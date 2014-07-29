#ifndef _APE_NET_CONNECTION_H_
#define _APE_NET_CONNECTION_H_

#include "handleralloc.h"
#include "buffer.h"
#include "baseparser.h"
#include "events.h"
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <string>
#include <deque>


namespace ape{
namespace net{
class CSession;
class CConnection : public boost::enable_shared_from_this<CConnection>, private boost::noncopyable {
public:
    typedef struct stLenMsg {
        int len;
        void *buf;
    }SLenMsg;
    CConnection(boost::asio::io_service &io_service, ape::protocol::ParserFactory *factory, CSession *o);
    ~CConnection();
    void AsyncConnect(const std::string &ip, unsigned int port, int timeout = 3);
    void AsyncRead();
    void AsyncWrite(ape::message::SNetMessage *msg, bool close = false);
    void HandleSendResponse(const std::string &strResponse);
    void OnPeerClose();
    void Dump();    
    void SetOwner(CSession *o) {session_ = o;}
    unsigned int Id(){return id_;}
    boost::asio::ip::tcp::socket& Socket(){return socket_;}
    const std::string & GetRemoteIp();
    unsigned int GetRemotePort();
    ape::protocol::CBaseParser *GetParser() const { return parser_;}
private:
    void HandleResolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void HandleConnected(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
    void HandleConnectTimeout(const boost::system::error_code& err);
    void ConnectFinish(int result);
    void AsyncReadInner();
    void HandleRead(const boost::system::error_code& err,std::size_t size);
    void HandleWrite(const boost::system::error_code& err);
private:
    boost::asio::io_service &io_service_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::deadline_timer conn_timer_;
    ape::common::CBuffer buffer_;
    unsigned int id_;
    static unsigned int sm_mark_;    
    std::string remoteip_;
    unsigned int remoteport_;
    ape::common::CHandlerAllocator conn_alloc_;
    ape::common::CHandlerAllocator timer_alloc_;
    ape::common::CHandlerAllocator read_alloc_;
    ape::common::CHandlerAllocator write_alloc_;
    bool connected_;
    bool close_after_write_;
    CSession *session_;
    ape::protocol::CBaseParser *parser_;
    std::deque<SLenMsg> out_queue_;
};

typedef boost::shared_ptr<CConnection> Connection_ptr;
}
}
#endif

