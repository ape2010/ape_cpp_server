#include "session.h"
#include "sessionmanager.h"
#include "threadtimer.h"
#include "errorcode.h"
#include "loghelper.h"
#include "protocolhelper.h"

namespace ape{
namespace net{
static const char *ConnectStatus[] = {"WAITING", "CONNECTED", "TIME_OUT", "CONNECTING", "CLOSED"};
const int RECONNECT_INTERVAL = 3000; //3s
CSession::CSession() : status_(WAITING), owner_(NULL), timer_owner_(NULL), port_(0), timer_reconn_(NULL), timer_heartbeat_(NULL)
{}
void CSession::Init(boost::asio::io_service &io, ape::protocol::EProtocolType pro, CSessionCallBack *o, ape::common::CTimerManager *tm, bool autoreconnect, int heartbeat) {
    proto_ = pro;
    ptrconn_.reset(new CConnection(io, pro, this));
    owner_ = o;
    timer_owner_ = tm;
    autoreconnect_ = autoreconnect;
    heartbeatinterval_ = heartbeat;
    //io.post(boost::bind(&CSession::OnAccept, this));
}
CSession::~CSession() {
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], request_history_.size[%u]\n", __FUNCTION__, Id(), request_history_.size());
    if (timer_heartbeat_) {
        timer_heartbeat_->Stop();
        delete timer_heartbeat_;
    }
    if (timer_reconn_) {
        timer_reconn_->Stop();
        delete timer_reconn_;
    }
    owner_ = NULL;
}
void CSession::Connect(const std::string &ip, unsigned int port) {
    ip_ = ip;
    port_ = port;
    DoConnect();
}
void CSession::ConnectResult(int result) {
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], addr[%s:%u], result[%d], autoreconn[%d], heartbeat[%d]\n",
        __FUNCTION__, Id(), ip_.c_str(), port_, result, autoreconnect_, heartbeatinterval_);
    if (result == 0) {
        status_ = CONNECTED;
        if (heartbeatinterval_ > 0 ) {
            if (NULL == timer_heartbeat_) {
                unsigned int interval = heartbeatinterval_ < 1000 ? 1000 : heartbeatinterval_;
                timer_heartbeat_ = new ape::common::CThreadTimer(timer_owner_, interval,
                    boost::bind(&CSession::DoHeartBeat, this), ape::common::CThreadTimer::TIMER_CIRCLE);
            }
            timer_heartbeat_->Start();
        }
        if (NULL != timer_reconn_) {
            timer_reconn_->Stop();
        }
    } else {
        if (autoreconnect_) {
            if (NULL == timer_reconn_) {
                timer_reconn_ = new ape::common::CThreadTimer(timer_owner_, RECONNECT_INTERVAL,
                    boost::bind(&CSession::DoConnect, this), ape::common::CThreadTimer::TIMER_ONCE);
            }
            timer_reconn_->Start();
        }
    }

    DealWaitingList();
}
void CSession::DoConnect() {
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], status_[%s], addr[%s:%u]\n", __FUNCTION__, Id(), ConnectStatus[status_], ip_.c_str(), port_);
	if (status_ == CONNECTED || status_ == CONNECTING) {
		return;
	}
	status_ = CONNECTING;
    ptrconn_->SetOwner(this);
    ptrconn_->AsyncConnect(ip_, port_);
}
void CSession::DoHeartBeat() {
    if (status_ != CONNECTED) {
        return;
    }
    ape::message::SNetMessage *msg = ptrconn_->GetParser()->CreateHeartBeatMessage();
    if (msg == NULL) {
        return;
    }
    //Dump();
    BS_XLOG(XLOG_TRACE,"CSession::%s, id[%u], addr[%s:%u]\n%s\n", __FUNCTION__, Id(), ip_.c_str(), port_, msg->NoticeInfo().c_str());
    ptrconn_->AsyncWrite(msg);
    delete msg;
}
void CSession::OnAccept() {
    owner_->OnAccept(this);
}
void CSession::OnConnected() {
    status_ = CONNECTED;
    owner_->OnConnected(this);
}
void CSession::OnPeerClose() {
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], addr[%s:%u], autoreconn[%d]\n",__FUNCTION__, Id(), GetRemoteIp().c_str(),
        GetRemotePort(), autoreconnect_);
    status_ = CLOSED;
    CleanRequestAndCallBack();
    if (autoreconnect_) {
        if (NULL == timer_reconn_) {
            timer_reconn_ = new ape::common::CThreadTimer(timer_owner_, RECONNECT_INTERVAL,
                boost::bind(&CSession::DoConnect, this), ape::common::CThreadTimer::TIMER_ONCE);
        }
		status_ = WAITING;
        timer_reconn_->Start();
    } else if (owner_) {
        owner_->OnPeerClose(this);
    }
}
void CSession::OnRead(ape::message::SNetMessage *msg) {
    if (msg->isheartbeat) {
        status_ = CONNECTED;
        if (msg->type == ape::message::SNetMessage::E_Request) {
            ape::message::SNetMessage *resmsg = ptrconn_->GetParser()->CreateHeartBeatMessage(ape::message::SNetMessage::E_Response);
            BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], addr[%s:%u]\n%s\n", __FUNCTION__, Id(), GetRemoteIp().c_str(), GetRemotePort(), resmsg->NoticeInfo().c_str());
            ptrconn_->AsyncWrite(resmsg);
            delete resmsg;
        }
        delete msg;
        return;
    }
    if (msg->type == ape::message::SNetMessage::E_Response) {
		if (msg->IsOk()) {status_ = CONNECTED;}
        unsigned int seqid = msg->GetSequenceId();
        BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], seqid[%d], request_history_.size[%u]\n", __FUNCTION__, Id(), seqid, request_history_.size());

        std::multimap<unsigned int, boost::shared_ptr<ape::common::CThreadTimer> >::iterator itr = request_history_.find(seqid);
        if (itr != request_history_.end()) {
            msg->ctx = ((ape::message::SNetMessage*)(itr->second->GetData()))->ctx;
            itr->second->Stop();
            itr->second->Callback(); //will call DoSendTimeOut
            if (owner_) {
                owner_->OnRead(this, msg);
                return;
            }
        }
    } else  if (owner_) {
    	status_ = CONNECTED;
        owner_->OnRead(this, msg);
        return;
    }
    delete msg;
}
void CSession::DoSendTo(void *para, int timeout) {
    ape::message::SNetMessage *msg = (ape::message::SNetMessage *)para;
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], status[%s], addr[%s:%u], timeout[%d], msg:\n%s\n", __FUNCTION__, Id(),
		ConnectStatus[status_], ip_.c_str(), port_, timeout, msg->NoticeInfo().c_str());

    if (status_ == CONNECTING || status_ == WAITING) {
        waitinglist_.push_back(SReadyPacket(para, timeout));
        return;
    }

    if (status_ != CONNECTED) {
        msg->SetReply(ape::common::ERROR_PEER_CLOSE);
        owner_->OnRead(this, msg);
        return;
    }
    DoSend(msg, timeout);
}
void CSession::DoSend(ape::message::SNetMessage *msg, int timeout) {
    boost::shared_ptr<ape::common::CThreadTimer> timer(new ape::common::CThreadTimer(timer_owner_, timeout,
            boost::bind(&CSession::DoSendTimeOut, this, msg), ape::common::CThreadTimer::TIMER_ONCE, msg));
    timer->Start();
    request_history_.insert(std::make_pair(msg->GetSequenceId(), timer));

    ptrconn_->AsyncWrite(msg, false);
}
void CSession::DoSendBack(void *para, bool close) {
    ape::message::SNetMessage *msg = (ape::message::SNetMessage *)para;
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], status_[%s], addr[%s:%u], close[%d] \n%s\n", __FUNCTION__, Id(),
		ConnectStatus[status_], GetRemoteIp().c_str(), GetRemotePort(), close, msg->NoticeInfo().c_str());
    ptrconn_->AsyncWrite(msg, close);
    delete msg;
}
void CSession::Close() {
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], status_[%s], addr[%s:%u]\n", __FUNCTION__, Id(),
		ConnectStatus[status_], GetRemoteIp().c_str(), GetRemotePort());
    status_ = CLOSED;
    ptrconn_->SetOwner(NULL);
    CleanRequestAndCallBack();
    owner_ = NULL;

    ptrconn_->OnPeerClose();
}

void CSession::DoSendTimeOut(void *para) {
    ape::message::SNetMessage *msg = (ape::message::SNetMessage *)para;
    unsigned int seqid = msg->GetSequenceId();
    std::multimap<unsigned int, boost::shared_ptr<ape::common::CThreadTimer> >::iterator itr = request_history_.find(seqid);
    if (itr != request_history_.end()) {
        if (owner_ && ape::common::CThreadTimer::TIME_OUT == itr->second->GetStatus()) {
            BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], addr[%s:%u], error[%d] \n%s\n", __FUNCTION__, Id(), ip_.c_str(), port_,
                ape::common::ERROR_TIME_OUT, msg->NoticeInfo().c_str());
            msg->SetReply(ape::common::ERROR_TIME_OUT);
            owner_->OnRead(this, msg);
        } else {
            delete msg;
        }
        request_history_.erase(itr);
    }
}
void CSession::CleanRequestAndCallBack() {
    while (!request_history_.empty()) {
        boost::shared_ptr<ape::common::CThreadTimer> timer = request_history_.begin()->second;
        timer->Stop();
        timer->Callback(); //will call DoSendTimeOut
        request_history_.erase(request_history_.begin());
    }
    while (!waitinglist_.empty()) {
        std::deque<SReadyPacket>::iterator itr =  waitinglist_.begin();
        ape::message::SNetMessage *msg = (ape::message::SNetMessage *)(itr->packet);
        waitinglist_.pop_front();
        msg->SetReply(ape::common::ERROR_PEER_CLOSE);
        owner_->OnRead(this, msg);
    }
}

void CSession::DealWaitingList() {
    while (!waitinglist_.empty()) {
        std::deque<SReadyPacket>::iterator itr =  waitinglist_.begin();
        ape::message::SNetMessage *msg = (ape::message::SNetMessage *)(itr->packet);
        waitinglist_.pop_front();
        BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], status[%s], send\n%s\n", __FUNCTION__, Id(), ConnectStatus[status_], msg->NoticeInfo().c_str());

        if (status_ == CONNECTED) {
            DoSend(msg, itr->timeout);
        } else {
            msg->SetReply(ape::common::ERROR_PEER_CLOSE);
            owner_->OnRead(this, msg);
        }
    }
}
void CSession::Dump() {
    BS_XLOG(XLOG_DEBUG,"CSession::%s, id[%u], request_history_.size[%u]\n", __FUNCTION__, Id(), request_history_.size());
}


}
}
