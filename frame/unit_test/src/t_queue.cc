#include "gtest/gtest.h"
#include "msgqueueprio.h"
#include "threadtimer.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_QUEUE
using namespace ape::common;

TEST(MsgQueuePrio, Dump) {
    MsgQueuePrio queue;
    EXPECT_EQ(0, queue.GetUsed());
    queue.PutQ((void *)0X001, 1);
    queue.PutQ((void *)0X002, 1);
    queue.Dump();
    EXPECT_EQ(2, queue.GetUsed());
    void *P = queue.GetQ();
    queue.Dump();
    //EXPECT_EQ(0X001, p);
    EXPECT_EQ(1, queue.GetUsed());
    P = queue.GetQ();
    //EXPECT_EQ(0X002, p);
    EXPECT_EQ(0, queue.GetUsed());
}
#endif
