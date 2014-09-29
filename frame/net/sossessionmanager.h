#ifndef _APE_NET_SOS_SESSION_MANAGER_H_
#define _APE_NET_SOS_SESSION_MANAGER_H_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include "ioservicethread.h"
#include "events.h"
#include "session.h"
#include "sessioncallback.h"
#include "netservice.h"

namespace ape {
namespace net {

class CSosSessionManager : public virtual CSessionCallBack, public virtual CNetService {
    class CSessionContainer;
    class CShortSessionContainer;
    class CPersistentSessionContainer;
 public:
    CSosSessionManager(boost::asio::io_service *io_service);
    virtual void OnConnected(void *session);
    virtual int  OnConnect(const std::string &name, const std::string &addr, bool autoreconnect = false, int heartbeat = 0);
    virtual int  DoConnect(const std::string &name, const std::string &addr, bool autoreconnect = false, int heartbeat = 0);
    virtual void OnSendTo(const std::string &name, void *para, int timeout = 30);
    virtual void DoSendTo(const std::string &name, void *para, int timeout = 30);
    virtual int  OnPeerClose(void *session);
    virtual ~CSosSessionManager();
    virtual void Dump();
    virtual void DoCheckSession();
    
 private:
    boost::asio::io_service *io_service_;
    CSessionContainer *short_sessions_;
    CSessionContainer *persistent_sessions_;
};
}
}
#endif

