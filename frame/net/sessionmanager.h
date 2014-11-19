#ifndef _APE_COMMON_SESSION_MANAGER_H_
#define _APE_COMMON_SESSION_MANAGER_H_
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <map>
#include "ioservicethread.h"
#include "events.h"
#include "sossessionmanager.h"
#include "socsessionmanager.h"
#include "session.h"

namespace ape {
namespace net {

class CSessionManager : public CIoServiceThread, public CSosSessionManager, public CSocSessionManager {
 public:
    CSessionManager(HandleFactory *f, int threadid);
    virtual ~CSessionManager();
    virtual void StartInThread();
    virtual void StopInThread();
    virtual void Dump();

    virtual boost::asio::io_service *GetIoService() {return CIoServiceThread::GetIoService();}
    virtual ape::common::CTimerManager *GetTimerManager() {return this;}
    virtual int GetThreadId() {return CIoServiceThread::GetThreadId();}
    virtual void OnRead(void *session, void *para);
    virtual int  OnPeerClose(void *session);

 private:
    void InitInner();
    void DoCheckSession();

 private:
    HandleFactory *factory;
    CHandle *handle_;
    ape::common::CThreadTimer tm_check_session_;
};
}
}
#endif

