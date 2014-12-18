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

MsgQueuePrio public_queue;
void *thread_run(void *)
{
    BS_XLOG(XLOG_DEBUG, "%s, \n", __FUNCTION__);
    while (1) {
        void *pdata = public_queue.GetQ(1000);
        //BS_XLOG(XLOG_DEBUG, "data[0X%0X]\n", pdata);
        if (pdata) {
            BS_XLOG(XLOG_DEBUG, "data[0X%0X]\n", pdata);
        }
    }
}
TEST(MsgQueuePrio, PutMessage) {
    pthread_t id[5];
    for (int i = 0; i < 1; ++i) {
        int ret = pthread_create(&id[i], NULL, &thread_run, NULL);
        if(ret!=0){
            BS_XLOG(XLOG_DEBUG, "Create pthread error!\n");
            exit (1);
        }
    }
    for (int i = 1; i < 10; ++i) {
        usleep(10);
        BS_XLOG(XLOG_DEBUG, "put [0X%0X]\n", i);
        public_queue.PutQ((void *)i, 1);
    }

    for(int i=0;i<5;i++){
        pthread_join(id[i],NULL);
    }
}

TEST(MsgQueuePrio, Sleep) {
    while(1) {
        sleep(3);
    }
}
#endif
