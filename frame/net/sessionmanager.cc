#include "sessionmanager.h"
#include "protocolhelper.h"
#include "loghelper.h"


namespace ape {
namespace net {
const int CHECK_SESSION_INTERVAL = 30000; //30s
CSessionManager::CSessionManager(HandleFactory *f, int threadid) : CIoServiceThread(threadid), 
    CNetService(), CSessionCallBack(), CSocSessionManager(&ioservice_), CSosSessionManager(&ioservice_), factory(f) {
    handle_ = factory->NewHandler();
    InitInner();
}
CSessionManager::~CSessionManager() {
    BS_XLOG(XLOG_TRACE,"CSessionManager::%s, threadid[%d]\n",__FUNCTION__, GetThreadId());
    handle_->Release();
}
void CSessionManager::InitInner() {
    tm_check_session_.Init(this, CHECK_SESSION_INTERVAL, boost::bind(&CSessionManager::DoCheckSession, this), ape::common::CThreadTimer::TIMER_CIRCLE);
}

void CSessionManager::StartInThread() {
    CIoServiceThread::StartInThread();
    tm_check_session_.Start();
    handle_->StartInThread(GetThreadId(), this, this);
}
void CSessionManager::StopInThread() {
    tm_check_session_.Stop();
    CIoServiceThread::StopInThread();
    handle_->StopInThread(GetThreadId());
}
int CSessionManager::OnPeerClose(void *session) {
    if (0 != CSocSessionManager::OnPeerClose(session)) {
        return CSosSessionManager::OnPeerClose(session);
    }
    return 0;
}
void CSessionManager::OnRead(void *session, void *para) {
    ape::message::SNetEvent *event = (ape::message::SNetEvent *)para;
    if (session) {event->connid = ((CSession*)session)->Id();}
    handle_->OnEvent(GetThreadId(), (ape::message::SEvent *)para);
}
void CSessionManager::DoCheckSession() {
    CSocSessionManager::DoCheckSession();
    CSosSessionManager::DoCheckSession();
}
void CSessionManager::Dump() {
    CIoServiceThread::Dump();
    CSocSessionManager::Dump();
    CSosSessionManager::Dump();
}
}
}
