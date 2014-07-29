#include "sessionmanager.h"
#include "protocolhelper.h"
#include "loghelper.h"


namespace ape {
namespace net {
const int CHECK_SESSION_INTERVAL = 30000; //30s
CSessionManager::CSessionManager(HandleFactory *f, int threadid) : CIoServiceThread(threadid), factory(f) {
    handle_ = factory->NewHandler();
    InitInner();
}
CSessionManager::~CSessionManager() {
    BS_XLOG(XLOG_TRACE,"CSessionManager::%s, threadid[%d]\n",__FUNCTION__, GetThreadId());
    std::map<unsigned int, CSession*>::iterator itr = soc_sessions_.begin();;
    for(; itr != soc_sessions_.end(); ++itr) {
        delete itr->second;
    }
    soc_sessions_.clear();
    
    boost::unordered_map<std::string, CSession*>::iterator itr2 = addr_sessions_.begin();;
    for(; itr2 != addr_sessions_.end(); ++itr2) {
        delete itr2->second;
    }
    addr_sessions_.clear();
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
/** name: monitor|rapid_1000|127.0.0.1:8008 ;addr: http://127.0.0.1:8008 */
int  CSessionManager::OnConnect(const std::string &name, const std::string &addr, bool autoreconnect, int heartbeat) {
    if (name.empty()) {
        BS_XLOG(XLOG_ERROR,"CSessionManager::%s, name is empty, addr[%s]\n",__FUNCTION__, addr.c_str());
        return -1;
    }
    char ip[32] = {0};
    unsigned int port;
    ape::protocol::EProtocol pro;
    if (0 != ape::protocol::ParseAddr(addr, &pro, ip, &port)) {
        BS_XLOG(XLOG_ERROR,"CSessionManager::%s, bad addr[%s]...\n",__FUNCTION__, addr.c_str());
        return -1;
    }
    GetIoService()->post(boost::bind(&CSessionManager::DoConnect, this, name, addr, autoreconnect, heartbeat));
    return 0;
}
int  CSessionManager::DoConnect(const std::string &name, const std::string &addr, bool autoreconnect, int heartbeat) {
    if (name.empty()) {
        BS_XLOG(XLOG_ERROR,"CSessionManager::%s, name is empty, addr[%s]\n",__FUNCTION__, addr.c_str());
        return -1;
    }
    CSession *session = NULL;
    boost::unordered_map<std::string, CSession*>::iterator itr = addr_sessions_.find(addr);
    if (itr != addr_sessions_.end()) {
        session = itr->second;
    } else {
        char ip[32] = {0};
        unsigned int port;
        ape::protocol::EProtocol pro;
        if (0 != ape::protocol::ParseAddr(addr, &pro, ip, &port)) {
            BS_XLOG(XLOG_ERROR,"CSessionManager::%s, bad addr[%s]...\n",__FUNCTION__, addr.c_str());
            return -1;
        }
        session = ape::protocol::CreateSession(pro);
        session->Init(*GetIoService(), pro, this, this, autoreconnect, heartbeat);
        session->SetAddr(addr);
        session->Connect(ip, port);
        addr_sessions_[addr] = session;
    }
    SSessionGroup &group = sos_session_group_[name];
    if (!binary_search(group.sessions.begin(), group.sessions.end(), addr)) {
        group.sessions.push_back(addr);
        sort(group.sessions.begin(), group.sessions.end());
    }
    return 0;
}
void CSessionManager::OnSendTo(const std::string &name, void *para, int timeout) {
    GetIoService()->post(boost::bind(&CSessionManager::DoSendTo, this, name, para, timeout));
}
void CSessionManager::DoSendTo(const std::string &name, void *para, int timeout) {
    boost::unordered_map<std::string, SSessionGroup>::iterator itr = sos_session_group_.find(name);
    if (itr == sos_session_group_.end()) {
        BS_XLOG(XLOG_WARNING,"CSessionManager::%s, no[%s]...\n",__FUNCTION__, name.c_str());
        ape::message::SNetMessage *msg = (ape::message::SNetMessage *)para;
        msg->SetReply(-1);
        handle_->OnEvent(GetThreadId(), msg);
        return;
    }
    SSessionGroup &group = itr->second;
    CSession* p = GetSession(&group);
    if (p == NULL) {
        BS_XLOG(XLOG_WARNING,"CSessionManager::%s, no session with the name[%s]...\n",__FUNCTION__, name.c_str());
        ape::message::SNetMessage *msg = (ape::message::SNetMessage *)para;
        msg->SetReply(-1);
        handle_->OnEvent(GetThreadId(), msg);
        return;
    }
    p->DoSendTo(para, timeout * 1000);
}
CSession *CSessionManager::GetSession(SSessionGroup *group) {
    if (group->sessions.empty()) {
        return NULL;
    }
    unsigned int size = group->sessions.size();
    for (unsigned int i = 0; i < size; ++i) {
        if(group->current >= group->sessions.size()) {
            group->current = 0;
        }
        boost::unordered_map<std::string, CSession*>::iterator itr;
        itr = addr_sessions_.find(group->sessions.at(group->current));
        if (itr != addr_sessions_.end()) {
            ++(group->current);
            return itr->second;
        }
        group->sessions.erase(group->sessions.begin() + group->current);
    }
    
    return NULL;
}
void CSessionManager::OnAccept(void *session) {
    CSession *p = (CSession*)session;
    soc_sessions_[p->Id()] = p;
}
void CSessionManager::OnConnected(void *session) {
    CSession *p = (CSession*)session;
    BS_XLOG(XLOG_DEBUG,"CSessionManager::%s, addr[%s:%u]\n",__FUNCTION__, p->GetRemoteIp().c_str(), p->GetRemotePort());
    soc_sessions_[p->Id()] = p;
}
void CSessionManager::OnPeerClose(void *session) {
    CSession *p = (CSession*)session;
    BS_XLOG(XLOG_DEBUG,"CSessionManager::%s, addr[%s:%u]\n",__FUNCTION__, p->GetRemoteIp().c_str(), p->GetRemotePort());
    /** todo notify onevent */
    const std::string &addr = p->GetAddr();
    if (addr.empty()) { //server-session
        std::map<unsigned int, CSession*>::iterator itr = soc_sessions_.find(p->Id());
        if(itr != soc_sessions_.end()) {
            delete itr->second;
            soc_sessions_.erase(itr);
            return;
        }
    } else { //client-session
        boost::unordered_map<std::string, CSession*>::iterator itr = addr_sessions_.find(addr);
        if(itr != addr_sessions_.end()) {
            delete itr->second;
            addr_sessions_.erase(itr);
            return;
        }
    }
}
void CSessionManager::OnRead(void *session, void *para) {
    ape::message::SNetEvent *event = (ape::message::SNetEvent *)para;
    event->connid = ((CSession*)session)->Id();
    handle_->OnEvent(GetThreadId(), (ape::message::SEvent *)para);
}
void CSessionManager::OnSendBack(unsigned int connid, void *para) {
    GetIoService()->post(boost::bind(&CSessionManager::DoSendBack, this, connid, para));
}
void CSessionManager::DoSendBack(unsigned int connid, void *para) {
    std::map<unsigned int, CSession*>::iterator itr = soc_sessions_.find(connid);
    if(itr != soc_sessions_.end()) {
        itr->second->DoSendBack(para);
    } else {
        BS_XLOG(XLOG_WARNING,"CSessionManager::%s, connection has be closed, message[%s]\n",__FUNCTION__, ((ape::message::SEvent *)para)->NoticeInfo().c_str());
    }
}

void CSessionManager::DoCheckSession() {
    //BS_XLOG(XLOG_DEBUG,"CSessionManager::%s, soc_sessions_.size[%u]...\n",__FUNCTION__, soc_sessions_.size());
    std::map<unsigned int, CSession*>::iterator itr = soc_sessions_.begin();;
    while(itr != soc_sessions_.end()) {
        std::map<unsigned int, CSession*>::iterator tmp = itr++;
        CSession *p = tmp->second;
        if(p == NULL){ continue;}
        CSession::EStatus status = p->GetStatus();
        if(status == CSession::CONNECTED){
            p->SetStatus(CSession::TIME_OUT);
        } else if(status == CSession::TIME_OUT || status == CSession::CLOSED) {
            /** todo notify OnEvent **/
            p->SetOwner(NULL);
            p->Close();
            soc_sessions_.erase(tmp);
            delete p;
        }
    }
}

void CSessionManager::Dump() {
    CIoServiceThread::Dump();
}
}
}
