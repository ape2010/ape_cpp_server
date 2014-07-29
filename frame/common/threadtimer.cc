#include "threadtimer.h"
#include <boost/bind.hpp>

namespace ape {
namespace common {
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