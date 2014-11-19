#include "sossessionmanager.h"
#include "protocolhelper.h"
#include "loghelper.h"


namespace ape {
namespace net {
class CSosSessionManager::CSessionContainer {
  public:
    virtual CSession *GetSession(const std::string &name) = 0;
    virtual CSession *GetSession(const std::string &name, const std::string &addr) = 0;
    virtual void AddSession(const std::string &name, const std::string &addr, CSession *session) = 0;
    virtual int  DeleteSession(CSession *session) = 0;
    virtual void CheckSession() = 0;
    virtual void Dump() = 0;
    virtual ~CSessionContainer() {}
};

class CSosSessionManager::CShortSessionContainer : public CSosSessionManager::CSessionContainer {
  public:
    virtual CSession *GetSession(const std::string &name) {
        boost::unordered_map<std::string, CSession*>::iterator itr = sessions_.find(name);;
        return itr == sessions_.end() ? NULL : itr->second;
    }
    virtual CSession *GetSession(const std::string &name, const std::string &addr) {
        return GetSession(name);
    }
    virtual void AddSession(const std::string &name, const std::string &addr, CSession *session) {
        boost::unordered_map<std::string, CSession*>::iterator itr = sessions_.find(name);;
        if (itr != sessions_.end()) {
            delete session;
            return;
        }
        sessions_[name] = session;
    }
    virtual int DeleteSession(CSession *session) {
        boost::unordered_map<std::string, CSession*>::iterator itr = sessions_.begin();;
        for(; itr != sessions_.end(); ++itr) {
            if (itr->second == session) {
                delete session;
                sessions_.erase(itr);
                return 0;
            }
        }
        return -1;
    }
    virtual void CheckSession() {
    	BS_XLOG(XLOG_DEBUG,"CShortSessionContainer::%s, sessions_.size[%u]\n",__FUNCTION__,sessions_.size());
        boost::unordered_map<std::string, CSession*>::iterator itr = sessions_.begin();;
        while(itr != sessions_.end()) {
            boost::unordered_map<std::string, CSession*>::iterator tmp = itr++;
            CSession *p = tmp->second;
            CSession::EStatus status = p->GetStatus();
            if(status == CSession::CONNECTED){
                p->SetStatus(CSession::TIME_OUT);
            } else if(status == CSession::TIME_OUT || status == CSession::CLOSED) {
                p->Close();  /** will flush history request and notify OnEvent **/
                sessions_.erase(tmp);
                delete p;
            }
        }
    }
    virtual void Dump() {
        BS_XLOG(XLOG_TRACE,"  CShortSessionContainer::%s, sessions_.size[%u]\n",__FUNCTION__,
            sessions_.size());
    }
    virtual ~CShortSessionContainer() {
        boost::unordered_map<std::string, CSession*>::iterator itr = sessions_.begin();;
        for(; itr != sessions_.end(); ++itr) {
            delete itr->second;
        }
        sessions_.clear();
    }
  private:
    boost::unordered_map<std::string, CSession*> sessions_;
};

class CSosSessionManager::CPersistentSessionContainer : public CSosSessionManager::CSessionContainer {
    typedef struct stSessionGroup{
        std::vector<std::string> addrs;
        unsigned int current;
        stSessionGroup():current(0){}
    }SSessionGroup;
  public:
    virtual CSession *GetSession(const std::string &name) {
        boost::unordered_map<std::string, SSessionGroup>::iterator itr = session_groups_.find(name);
        if (itr == session_groups_.end()) {
            return NULL;
        }
        SSessionGroup &group = itr->second;
        if (group.addrs.empty()) {
            return NULL;
        }
        unsigned int size = group.addrs.size();
        for (unsigned int i = 0; i < size; ++i) {
            if(group.current >= size) {
                group.current = 0;
            }
            boost::unordered_map<std::string, CSession*>::iterator itrs;
            itrs = addr_sessions_.find(group.addrs.at(group.current));
            if (itrs != addr_sessions_.end()) {
                ++(group.current);
                return itrs->second;
            }
        }
        return NULL;
    }
    virtual CSession *GetSession(const std::string &name, const std::string &addr) {
        boost::unordered_map<std::string, CSession*>::iterator itr = addr_sessions_.find(addr);
        if (itr != addr_sessions_.end()) {
            return itr->second;
        }
        return NULL;
    }
    virtual void AddSession(const std::string &name, const std::string &addr, CSession *session) {
        boost::unordered_map<std::string, CSession*>::iterator itr = addr_sessions_.find(addr);
        if (itr == addr_sessions_.end()) {
            addr_sessions_[addr] = session;
        }

        SSessionGroup &group = session_groups_[name];
        if (!binary_search(group.addrs.begin(), group.addrs.end(), addr)) {
            group.addrs.push_back(addr);
            sort(group.addrs.begin(), group.addrs.end());
        }
    }
    virtual int DeleteSession(CSession *session) {
        boost::unordered_map<std::string, CSession*>::iterator itr = addr_sessions_.begin();
        for(; itr != addr_sessions_.end(); ++itr) {
            if (itr->second == session) {
                delete session;
                addr_sessions_.erase(itr);
                return 0;
            }
        }
        return -1;
    }
    virtual void CheckSession() {
    	BS_XLOG(XLOG_DEBUG,"CPersistentSessionContainer::%s, addr_sessions_.size[%u], session_groups_.size[%u]\n",__FUNCTION__,
			addr_sessions_.size(), session_groups_.size());
		/*
		boost::unordered_map<std::string, CSession*>::iterator itrs = addr_sessions_.begin();;
        while(itrs != addr_sessions_.end()) {
            boost::unordered_map<std::string, CSession*>::iterator tmp = itrs++;
            CSession *p = tmp->second;
            CSession::EStatus status = p->GetStatus();
            if(status == CSession::CONNECTED){
                p->SetStatus(CSession::TIME_OUT);
            } else if(status == CSession::TIME_OUT || status == CSession::CLOSED) {
                p->Close();  // will flush history request and notify OnEvent
                addr_sessions_.erase(tmp);
                delete p;
            }
        }

        boost::unordered_map<std::string, SSessionGroup>::iterator itr = session_groups_.begin();
        for(; itr != session_groups_.end(); ++itr) {
            SSessionGroup &group = itr->second;
            std::vector<std::string>::iterator itr_addr = group.addrs.begin();
            while (itr_addr != group.addrs.end()) {
                boost::unordered_map<std::string, CSession*>::iterator itrm;
                itrm = addr_sessions_.find(*itr_addr);
                if (itrm == addr_sessions_.end()) {
                    itr_addr = group.addrs.erase(itr_addr);
                } else {
                    ++itr_addr;
                }
            }
        }
        */
    }
    virtual void Dump() {
        BS_XLOG(XLOG_TRACE,"  CPersistentSessionContainer::%s, addr_sessions_.size[%u], session_groups_.size[%u]\n",__FUNCTION__,
            addr_sessions_.size(), session_groups_.size());
    }
    virtual ~CPersistentSessionContainer() {
        boost::unordered_map<std::string, CSession*>::iterator itr = addr_sessions_.begin();;
        for(; itr != addr_sessions_.end(); ++itr) {
            delete itr->second;
        }
        addr_sessions_.clear();
        session_groups_.clear();
    }
  private:
    boost::unordered_map<std::string, CSession*> addr_sessions_;
    boost::unordered_map<std::string, SSessionGroup> session_groups_;
};

/********************************************************************************************/
CSosSessionManager::CSosSessionManager(boost::asio::io_service *io_service) : CNetService(), CSessionCallBack(), io_service_(io_service)  {
    short_sessions_ = new CShortSessionContainer;
    persistent_sessions_ = new CPersistentSessionContainer;
}
CSosSessionManager::~CSosSessionManager() {
    BS_XLOG(XLOG_TRACE,"CSosSessionManager::%s\n",__FUNCTION__);
    delete short_sessions_;
    delete persistent_sessions_;
}

/** name: monitor|rapid_1000|127.0.0.1:8008 ;addr: http://127.0.0.1:8008 */
int  CSosSessionManager::OnConnect(const std::string &name, const std::string &addr, bool autoreconnect, int heartbeat) {
    io_service_->post(boost::bind(&CSosSessionManager::DoConnect, this, name, addr, autoreconnect, heartbeat));
    return 0;
}
int  CSosSessionManager::DoConnect(const std::string &name, const std::string &addr, bool autoreconnect, int heartbeat) {
    BS_XLOG(XLOG_DEBUG,"CSosSessionManager::%s, name[%s], addr[%s], autoreconnect[%d], heartbeat[%d]\n",__FUNCTION__,
        name.c_str(), addr.c_str(), autoreconnect, heartbeat);
    if (name.empty()) {
        BS_XLOG(XLOG_ERROR,"CSosSessionManager::%s, name is empty, addr[%s]\n",__FUNCTION__, addr.c_str());
        return -1;
    }
    CSessionContainer *session_container = (!autoreconnect && heartbeat == 0) ? short_sessions_ : persistent_sessions_;
    if (NULL != session_container->GetSession(name, addr)) {
        return 0;
    }

    char ip[32] = {0};
    unsigned int port;
    ape::protocol::EProtocolType pro;
    if (0 != ape::protocol::ParseAddr(addr.c_str(), &pro, ip, &port)) {
        BS_XLOG(XLOG_ERROR,"CSosSessionManager::%s, bad addr[%s]...\n",__FUNCTION__, addr.c_str());
        return -1;
    }
    CSession *session = ape::net::SessionFactory::CreateSession(pro);
    session->Init(*io_service_, pro, this, GetTimerManager(), autoreconnect, heartbeat);
    session->SetAddr(addr);
    session->Connect(ip, port);

    session_container->AddSession(name, addr, session);
    return 0;
}
void CSosSessionManager::OnSendTo(const std::string &name, void *para, int timeout) {
    io_service_->post(boost::bind(&CSosSessionManager::DoSendTo, this, name, para, timeout));
}
void CSosSessionManager::DoSendTo(const std::string &name, void *para, int timeout) {
    CSession *session = short_sessions_->GetSession(name);
    if (session == NULL) {
        session = persistent_sessions_->GetSession(name);
    }
    if (session == NULL) {
        BS_XLOG(XLOG_WARNING,"CSosSessionManager::%s, no session[%s]...\n",__FUNCTION__, name.c_str());
        ape::message::SNetMessage *msg = (ape::message::SNetMessage *)para;
        msg->SetReply(-1);
        OnRead(NULL, msg);
        return;
    }

    session->DoSendTo(para, timeout * 1000);
}

void CSosSessionManager::OnConnected(void *session) {
    CSession *p = (CSession*)session;
    BS_XLOG(XLOG_DEBUG,"CSosSessionManager::%s, addr[%s:%u]\n",__FUNCTION__, p->GetRemoteIp().c_str(), p->GetRemotePort());
}
int CSosSessionManager::OnPeerClose(void *session) {
    CSession *p = (CSession*)session;
    BS_XLOG(XLOG_DEBUG,"CSosSessionManager::%s, addr[%s:%u]\n",__FUNCTION__, p->GetRemoteIp().c_str(), p->GetRemotePort());

    if (0 != short_sessions_->DeleteSession(p)) {
        return persistent_sessions_->DeleteSession(p);
    }
    return 0;
}

void CSosSessionManager::DoCheckSession() {
    BS_XLOG(XLOG_DEBUG,"CSosSessionManager::%s\n",__FUNCTION__);
    short_sessions_->CheckSession();
    persistent_sessions_->CheckSession();
}

void CSosSessionManager::Dump() {
    BS_XLOG(XLOG_TRACE,"CSosSessionManager::%s\n",__FUNCTION__);
    short_sessions_->Dump();
    persistent_sessions_->Dump();
}
}
}
