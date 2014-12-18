#include "gtest/gtest.h"
#include "timermanager.h"
#include "testthread.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_Thread
using namespace ape::common;

TEST(TestTimerThread, Dump) {
    TestThreadGroup::GetInstance()->Start(3);
    TestThreadGroup::GetInstance()->OnPrivateMsg();
    for (int i = 0; i < 10; ++i) {
        TestThreadGroup::GetInstance()->OnPublicMsg(i);
        //sleep(1);
    }
}
TEST(TestTimerThread, Sleep) {
    sleep(3);
}

#endif
