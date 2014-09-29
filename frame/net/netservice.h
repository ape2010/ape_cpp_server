#ifndef _APE_COMMON_NET_SERVICE_H_
#define _APE_COMMON_NET_SERVICE_H_
#include <boost/asio.hpp>
#include "timermanager.h"
namespace ape {
namespace net {
class CNetService {
 public:
    virtual boost::asio::io_service *GetIoService() = 0;
    virtual ape::common::CTimerManager *GetTimerManager() = 0;
    virtual int  OnConnect(const std::string &name, const std::string &addr, bool autoreconnect = false, int heartbeat = 0) = 0;
    virtual int  DoConnect(const std::string &name, const std::string &addr, bool autoreconnect = false, int heartbeat = 0) = 0;
    virtual void OnSendTo(const std::string &name, void *para, int timeout = 30) = 0;
    virtual void DoSendTo(const std::string &name, void *para, int timeout = 30) = 0;
    virtual void OnSendBack(unsigned int connid, void *para) = 0;
    virtual void DoSendBack(unsigned int connid, void *para) = 0;
};
class CHandle {
 public:
    virtual void StartInThread(int threadid, CNetService *service, ape::common::CTimerManager *owner) = 0;
    virtual void StopInThread(int threadid) = 0;
    virtual void OnEvent(int threadid, void *event) = 0;
    virtual void Release() {delete this;}
    virtual ~CHandle(){}
};
class HandleFactory {
 public:
    virtual CHandle* NewHandler() = 0;
    virtual ~HandleFactory() {}
};
template <class HANDLER>
class NetHandleFactory : public HandleFactory {
 public:
    virtual CHandle* NewHandler() {
        return new HANDLER();
    }
    virtual ~NetHandleFactory() {}
};
}
}
#endif
