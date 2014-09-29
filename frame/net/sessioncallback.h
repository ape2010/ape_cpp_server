#ifndef _APE_NET_SESSION_CALLBACK_H_
#define _APE_NET_SESSION_CALLBACK_H_
#include <boost/asio.hpp>
namespace ape {
namespace net {
class CSessionCallBack {
 public:
    virtual boost::asio::io_service *GetIoService() = 0;
    virtual int  GetThreadId() = 0;
    virtual void OnAccept(void *session) = 0;
    virtual void OnConnected(void *session) = 0;
    virtual int  OnPeerClose(void *session) = 0;
    virtual void OnRead(void *session, void *para) = 0;
    virtual ~CSessionCallBack() {}
};
class CNetServiceHolder {
 public:
    virtual CSessionCallBack *GetSessionCallBack() = 0;
    virtual ~CNetServiceHolder() {}
};
}
}
#endif
