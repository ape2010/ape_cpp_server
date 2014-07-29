#include "gtest/gtest.h"
#include "timermanager.h"
#include "testthread.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_Thread
using namespace ape::common;

TEST(TestTimerThread, Dump) {
    TestThreadGroup::GetInstance()->Start();
    for (int i = 0; i < 5; ++i) {
        TestThreadGroup::GetInstance()->OnPublicMsg();
        TestThreadGroup::GetInstance()->OnPrivateMsg();
        sleep(1);
    }
}
#endif
