#include <gtest/gtest.h>
#include "timermanager.h"
#include "threadtimer.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_Timer
using namespace ape::common;

void callback1() { BS_XLOG(XLOG_DEBUG, "----%s--\n", __FUNCTION__);}
void callback1_1() { BS_XLOG(XLOG_DEBUG, "----%s--\n", __FUNCTION__);}
void callback2() { BS_XLOG(XLOG_DEBUG, "----%s--\n", __FUNCTION__);}
void callback3() { BS_XLOG(XLOG_DEBUG, "----%s--\n", __FUNCTION__);}
void callback4(void *p) { BS_XLOG(XLOG_DEBUG, "----%s--, p[%p]--\n", __FUNCTION__, p);}

TEST(CTimerManager, Dump) {
    CTimerManager timermanager;
    CThreadTimer timer1(&timermanager, 1000, (boost::function<void ()>)(&callback1), CThreadTimer::TIMER_CIRCLE);
    CThreadTimer timer1_1(&timermanager, 1001, (boost::function<void ()>)(&callback1_1));
    CThreadTimer timer2(&timermanager, 2000, (boost::function<void ()>)(&callback2));
    CThreadTimer timer3(&timermanager, 3000, (boost::function<void ()>)(&callback3), CThreadTimer::TIMER_CIRCLE);
    CThreadTimer timer4(&timermanager, (unsigned int)2550, (boost::function<void ()>)(boost::bind(&callback4, (void*)0X001)), CThreadTimer::TIMER_CIRCLE);
    timer1.Start();
    timer1_1.Start();
    timer2.Start();
    timer3.Start();
    timer4.Start();    
    timermanager.Dump();
    
    timer2.Stop();    
    timermanager.Dump();
    
    timer1.Stop();    
    timer3.Stop(); 
    timermanager.Dump();
    
    timer1.Start();    
    timer3.Start(); 
    timer2.Start();
    timermanager.Dump();
    timer1.Start();  
    
    for (int i = 0; i < 10; ++i) {
        timermanager.DetectTimerList();
        //usleep(100000);
        sleep(1);
        
        //timermanager.Dump();
    }
}
#endif
