#include "threadtimer.h"
#include <boost/bind.hpp>
#include "loghelper.h"

namespace ape {
namespace common {
CThreadTimer::~CThreadTimer() {
    //BS_XLOG(XLOG_DEBUG, "CTimerManager::%s, interval[%u], entype_[%d], status[%u]\n", __FUNCTION__, dwinterval_, entype_, status_);
}
bool CThreadTimer::Start() {
    Stop();
    status_ = TIME_OUT;
    timernode_ = powner_->AddTimer(dwinterval_, this);
    return timernode_ == NULL ? false : true;
}
void CThreadTimer::Stop() {
    status_ = USER_CANCLE;
    if (timernode_) {
        powner_->RemoveTimer(timernode_);
        timernode_ = NULL;
    }
}
void CThreadTimer::Callback() {
    timernode_ = NULL;
    if (entype_ == TIMER_CIRCLE) {
        Start();
    }
    callback_(); //must at the last line; timer may be deleted in callback_();
}
}
}