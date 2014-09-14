#ifndef _APE_COMMON_SESSION_H_
#define _APE_COMMON_SESSION_H_

#include "connection.h"
#include "netservice.h"
#include "protocol.h"
#include "timermanager.h"

namespace ape{
namespace net{
typedef boost::shared_ptr<CConnection> Connection_ptr;
class CSession {
 public:
    typedef enum { WAITING = 0, CONNECTED, TIME_OUT, CONNECTING, CLOSED } EStatus;
    CSession();
    virtual void Init(boost::asio::io_service &io, ape::protocol::EProtocol pro, CNetService *o, 
        ape::common::CTimerManager *tm = NULL, bool autoreconnect = false, int heartbeat = 0);
    virtual void Connect(const std::string &ip, unsigned int port);
    virtual void ConnectResult(int result);
    virtual void OnAccept();
    virtual void OnConnected();
    virtual void OnPeerClose();
    virtual void OnRead(ape::message::SNetMessage *msg);
    virtual void DoSendTo(void *para, int timeout);
    virtual void DoSendBack(void *para, bool close = false);
    virtual void Close();
    virtual void Dump();
    virtual ~CSession();

    void SetOwner(CNetService *o) {owner_ = o;}
    CNetService *GetOwner(){return owner_;}
    void SetTimerManager(ape::common::CTimerManager *o) {timer_owner_ = o;}
    ape::common::CTimerManager *GetTimerManager(){return timer_owner_;}
    unsigned int Id() {return ptrconn_->Id();}
    Connection_ptr GetConnectPrt() const {return ptrconn_;}
    void SetAddr(const std::string &addr){addr_ = addr;}
    const std::string &GetAddr(){return addr_;}
    const std::string &GetRemoteIp(){return ptrconn_->GetRemoteIp();}
    unsigned int GetRemotePort(){return ptrconn_->GetRemotePort();}
    EStatus GetStatus(){return status_;}
    void SetStatus(EStatus s){status_ = s;}
 private:
    void DealWaitingList();
    void DoSendTimeOut(void *para);
    void CleanRequestTimers();
    void DoConnect();
    void DoHeartBeat();
private:
    EStatus status_;
    ape::protocol::EProtocol proto_;
    Connection_ptr ptrconn_;
    CNetService *owner_;
    ape::common::CTimerManager *timer_owner_;
    std::string addr_;
    std::string ip_;
    unsigned int port_;
    bool autoreconnect_;
    int heartbeatinterval_;
    ape::common::CThreadTimer *timer_reconn_;
    ape::common::CThreadTimer *timer_heartbeat_;
    
    std::multimap<unsigned int, boost::shared_ptr<ape::common::CThreadTimer> > request_history_;
    std::deque<void *> waitinglist_;
    
};
//typedef boost::shared_ptr<CSession> Session_Ptr;
}
}
#endif

