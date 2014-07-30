#ifndef APE_COMMON_THREAD_TIMER_H_
#define APE_COMMON_THREAD_TIMER_H_

#include "timermanager.h"
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <stdint.h>
namespace ape {
namespace common {
class CThreadTimer {
 public:
    typedef enum { TIMER_ONCE=0, TIMER_CIRCLE }ETimerType;
    typedef enum { WAITING=0, TIME_OUT, USER_CANCLE}EStatus;
    CThreadTimer() : powner_(NULL), status_(WAITING), data_(NULL), timernode_(NULL){}
    template <typename Func>
    CThreadTimer(CTimerManager *o, uint32_t i=0, Func f=0, ETimerType t=TIMER_ONCE, void *data=NULL) : 
            powner_(o), dwinterval_(i), entype_(t), status_(WAITING), callback_(f), timernode_(NULL), data_(data) { }
    template <typename Func>
    void Init(CTimerManager *o, uint32_t i=0, Func f=0, ETimerType t=TIMER_ONCE, void *data=NULL) {
        powner_ = o;
        dwinterval_ = i;
        callback_ = f;
        entype_ = t;
        data_ = data;
    }
    bool Start();
    void Stop();
    int GetStatus(){return status_;}
    void SetData(void *data) {data_ = data;}
    void *GetData() {return data_;}
  
 public:
    void Callback();
    void SetInterval(uint32_t i) { dwinterval_ = i;}
    uint32_t GetInterval() { return dwinterval_;}
  
 private:
    CTimerManager *powner_;
    uint32_t dwinterval_; //unit : ms
    ETimerType entype_;
    EStatus status_;

    boost::function<void ()> callback_;
    STimerNode *timernode_;
    void *data_;
};
}
}
#endif
