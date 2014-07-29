#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <arpa/inet.h>

#include "ioservicethread.h"
#include "loghelper.h"

namespace ape {
namespace net {

const int SELF_CHECK_INTERVAL = 2000;

CIoServiceThread::CIoServiceThread():threadid_(0), isalive_(false), dealline_timer_(ioservice_){
    InitInner();
}
CIoServiceThread::CIoServiceThread(int threadid):threadid_(threadid), isalive_(false), dealline_timer_(ioservice_) {
    InitInner();
}
CIoServiceThread::~CIoServiceThread() {
}
void CIoServiceThread::InitInner() {
    tm_selfcheck_.Init(this, SELF_CHECK_INTERVAL, boost::bind(&CIoServiceThread::DoSelfCheck, this), ape::common::CThreadTimer::TIMER_CIRCLE);
}
void CIoServiceThread::Start() {    
    BS_XLOG(XLOG_DEBUG,"CIoServiceThread::%s, threadid_[%d]\n",__FUNCTION__, threadid_);
    work_ = new boost::asio::io_service::work(ioservice_);
    thread_ = boost::thread(boost::bind(&boost::asio::io_service::run, &ioservice_)); 
    ioservice_.post(boost::bind(&CIoServiceThread::StartInThread, this));

    isalive_ = true;
    dealline_timer_.expires_from_now(boost::posix_time::microseconds(200000));
    dealline_timer_.async_wait(ape::common::MakeAllocHandler(alloc_timer, boost::bind(&CIoServiceThread::DoTimer, this)));
}
void CIoServiceThread::StartInThread() {
    BS_XLOG(XLOG_DEBUG,"CIoServiceThread::%s, threadid_[%d]\n",__FUNCTION__, threadid_);
    isalive_ = true;
    tm_selfcheck_.Start();
}
void CIoServiceThread::StopInThread() {
    BS_XLOG(XLOG_DEBUG,"CIoServiceThread::%s, threadid_[%d]\n",__FUNCTION__, threadid_);
    tm_selfcheck_.Stop();
}
void CIoServiceThread::DoTimer() {
    //BS_XLOG(XLOG_TRACE,"CIoServiceThread::%s\n",__FUNCTION__);
    DetectTimerList();
    if(isalive_) {
        dealline_timer_.expires_from_now(boost::posix_time::microseconds(200000));
        dealline_timer_.async_wait(ape::common::MakeAllocHandler(alloc_timer, boost::bind(&CIoServiceThread::DoTimer, this)));
    }
}
void CIoServiceThread::Stop() {
    BS_XLOG(XLOG_DEBUG,"CIoServiceThread::%s, threadid_[%d]\n",__FUNCTION__, threadid_);
    isalive_ = false;
    ioservice_.post(boost::bind(&CIoServiceThread::StopInThread, this));
    delete work_;
    thread_.join();
    ioservice_.reset();
}
void CIoServiceThread::GetSelfCheck(bool &isalive) {
    isalive = isalive_;
    isalive_ = false;
}
void CIoServiceThread::DoSelfCheck() {
    //BS_XLOG(XLOG_DEBUG,"CIoServiceThread::%s\n",__FUNCTION__);
    isalive_ = true;
}
void CIoServiceThread::Dump() {
    CTimerManager::Dump();
}
}
}


