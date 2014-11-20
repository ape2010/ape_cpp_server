#include "socsessionmanager.h"
#include "protocolhelper.h"
#include "loghelper.h"


namespace ape {
namespace net {

CSocSessionManager::~CSocSessionManager() {
    BS_XLOG(XLOG_TRACE,"CSocSessionManager::%s\n",__FUNCTION__);
    std::map<unsigned int, CSession*>::iterator itr = sessions_.begin();;
    for(; itr != sessions_.end(); ++itr) {
        delete itr->second;
    }
    sessions_.clear();
}

void CSocSessionManager::OnAccept(void *session) {
    CSession *p = (CSession*)session;
    BS_XLOG(XLOG_DEBUG,"CSocSessionManager::%s, sessionid[%d], addr[%s:%u]\n",__FUNCTION__, p->Id(), p->GetRemoteIp().c_str(), p->GetRemotePort());
    sessions_[p->Id()] = p;
}

int CSocSessionManager::OnPeerClose(void *session) {
    CSession *p = (CSession*)session;
    BS_XLOG(XLOG_DEBUG,"CSocSessionManager::%s, addr[%s:%u]\n",__FUNCTION__, p->GetRemoteIp().c_str(), p->GetRemotePort());

    std::map<unsigned int, CSession*>::iterator itr = sessions_.find(p->Id());
    if(itr != sessions_.end()) {
        delete itr->second;
        sessions_.erase(itr);
        return 0;
    }
    return -1;
}

void CSocSessionManager::OnSendBack(unsigned int connid, void *para) {
    io_service_->post(boost::bind(&CSocSessionManager::DoSendBack, this, connid, para));
}
void CSocSessionManager::DoSendBack(unsigned int connid, void *para) {
    std::map<unsigned int, CSession*>::iterator itr = sessions_.find(connid);
    if(itr != sessions_.end()) {
        itr->second->DoSendBack(para);
    } else {
        BS_XLOG(XLOG_WARNING,"CSocSessionManager::%s, connection has be closed, message[%s]\n",__FUNCTION__, ((ape::message::SEvent *)para)->NoticeInfo().c_str());
    }
}

void CSocSessionManager::DoCheckSession() {
    //BS_XLOG(XLOG_TRACE,"CSocSessionManager::%s, sessions_.size[%u]...\n",__FUNCTION__, sessions_.size());
    std::map<unsigned int, CSession*>::iterator itr = sessions_.begin();;
    while(itr != sessions_.end()) {
        std::map<unsigned int, CSession*>::iterator tmp = itr++;
        CSession *p = tmp->second;
        if(p == NULL){ continue;}
        CSession::EStatus status = p->GetStatus();
        if(status == CSession::CONNECTED){
            p->SetStatus(CSession::TIME_OUT);
        } else if(status == CSession::TIME_OUT || status == CSession::CLOSED) {
            p->Close();  /** will notify OnEvent **/
            sessions_.erase(tmp);
            delete p;
        }
    }
}

void CSocSessionManager::Dump() {
    BS_XLOG(XLOG_TRACE,"CSocSessionManager::%s, session_.size[%u]\n",__FUNCTION__, sessions_.size());
}
}
}
