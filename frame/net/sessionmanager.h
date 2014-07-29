#ifndef _APE_COMMON_SESSION_MANAGER_H_
#define _APE_COMMON_SESSION_MANAGER_H_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include "ioservicethread.h"
#include "events.h"
#include "session.h"

namespace ape {
namespace net {

class CSessionManager : public CIoServiceThread, public CNetService {
 private:
    typedef struct stSessionGroup{
        std::vector<std::string> sessions;
        unsigned int current;
        stSessionGroup():current(0){}
    }SSessionGroup;
 public:
    CSessionManager(HandleFactory *f, int threadid);
    virtual ~CSessionManager();
    virtual void StartInThread();
    virtual void StopInThread();
    //virtual void OnEvent(SEvent *event) = 0;
    virtual void Dump();
    
    virtual int  OnConnect(const std::string &name, const std::string &addr, bool autoreconnect = false, int heartbeat = 0);
    virtual int  DoConnect(const std::string &name, const std::string &addr, bool autoreconnect = false, int heartbeat = 0);
    virtual void OnSendTo(const std::string &name, void *para, int timeout = 30);
    virtual void DoSendTo(const std::string &name, void *para, int timeout = 30);
    
    virtual boost::asio::io_service *GetIoService(){return CIoServiceThread::GetIoService();}
    virtual void OnAccept(void *session);
    virtual void OnConnected(void *session);
    virtual void OnPeerClose(void *session);
    virtual void OnRead(void *session, void *para);
    virtual void OnSendBack(unsigned int connid, void *para);
    virtual void DoSendBack(unsigned int connid, void *para);
    
 private:
    void InitInner();
    void DoCheckSession();
    CSession *GetSession(SSessionGroup *group);
    
 private:
    HandleFactory *factory;
    CHandle *handle_;
    ape::common::CThreadTimer tm_check_session_;
    std::map<unsigned int, CSession*> soc_sessions_;
    boost::unordered_map<std::string, CSession*> addr_sessions_;
    boost::unordered_map<std::string, SSessionGroup> sos_session_group_;
};
}
}
#endif

