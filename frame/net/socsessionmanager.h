#ifndef _APE_NET_SOC_SESSION_MANAGER_H_
#define _APE_NET_SOC_SESSION_MANAGER_H_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include "events.h"
#include "session.h"
#include "sessioncallback.h"
#include "netservice.h"

namespace ape {
namespace net {

class CSocSessionManager : public virtual CNetService, public virtual CSessionCallBack {
 public:
    CSocSessionManager(boost::asio::io_service *io_service) : CNetService(), CSessionCallBack(), io_service_(io_service) {}
    virtual void OnAccept(void *session);
    virtual int  OnPeerClose(void *session);
    virtual void OnSendBack(unsigned int connid, void *para);
    virtual void DoSendBack(unsigned int connid, void *para);
    virtual void DoCheckSession();
    virtual void Dump();
    virtual ~CSocSessionManager();
 private:
    boost::asio::io_service *io_service_;
    std::map<unsigned int, CSession*> sessions_;
};
}
}
#endif

