#include "testthread.h"
#include "loghelper.h"

const int SELF_CHECK_INTERVAL = 3000; //3s
TestTimerThread::TestTimerThread(MsgQueuePrio *queue, int id) : MsgTimerThread(queue), id_(id),
      timerselfcheck_(this, SELF_CHECK_INTERVAL, boost::bind(&TestTimerThread::SelfCheck, this), CThreadTimer::TIMER_CIRCLE) {
}
void TestTimerThread::StartInThread() {
    BS_XLOG(XLOG_DEBUG,"TestTimerThread::%s, id[%d]\n", __FUNCTION__, id_);
    timerselfcheck_.Start();
}
void TestTimerThread::StopInThread() {
    BS_XLOG(XLOG_DEBUG,"TestTimerThread::%s, id[%d]\n", __FUNCTION__, id_);
    timerselfcheck_.Stop();
}
void TestTimerThread::Deal(void *pdata) {
    SMsg *msg = (SMsg *)pdata;
    BS_XLOG(XLOG_DEBUG,"TestTimerThread::%s, id[%d], msgid[%d]\n", __FUNCTION__, id_, msg->id);
    if (msg->id == 0) {
        DoPrivateMsg(msg);
    } else if (msg->id == 1) {
        DoPublicMsg(msg);
    }
    delete (SMsg *)pdata;
}
void TestTimerThread::DoPublicMsg(SMsg *msg) {
    BS_XLOG(XLOG_DEBUG,"TestTimerThread::%s, id[%d]\n", __FUNCTION__, id_);
}
void TestTimerThread::DoPrivateMsg(SMsg *msg) {
    BS_XLOG(XLOG_DEBUG,"TestTimerThread::%s, id[%d]\n", __FUNCTION__, id_);
}
void TestTimerThread::SelfCheck() {
    BS_XLOG(XLOG_DEBUG,"TestTimerThread::%s, id[%d]\n", __FUNCTION__, id_);
}

/************************************************************************************/
/***********************  TestThreadGroup  ******************************************/
/************************************************************************************/
TestThreadGroup *TestThreadGroup::sminstance_ = NULL;
TestThreadGroup *TestThreadGroup::GetInstance() {
    if (sminstance_ == NULL) {
        sminstance_ = new TestThreadGroup();
    }
    return sminstance_;
}
void TestThreadGroup::Release() {
    if(sminstance_) {
        delete sminstance_;
    }
}
TestThreadGroup::TestThreadGroup() {
}
TestThreadGroup::~TestThreadGroup() {
    Stop();
}
void TestThreadGroup::Start(int threads) {
    BS_XLOG(XLOG_DEBUG,"TestThreadGroup::%s, threads[%d]\n", __FUNCTION__, threads);
    for (int i = 0; i < threads; ++i) {
        TestTimerThread *thread = new TestTimerThread(&queue_, i);
        threads_.push_back(thread);
        thread->Start();
    }
}
void TestThreadGroup::Stop() {
    BS_XLOG(XLOG_DEBUG,"TestThreadGroup::%s\n", __FUNCTION__);
    std::vector<TestTimerThread *>::iterator itr;
    for (itr = threads_.begin(); itr != threads_.end(); ++itr) {
        (*itr)->Stop();
    }
    sleep(1);
    for (itr = threads_.begin(); itr != threads_.end(); ++itr) {
        delete (*itr);
    }
    threads_.clear();
}
void TestThreadGroup::OnPublicMsg() {
    BS_XLOG(XLOG_DEBUG,"TestThreadGroup::%s\n", __FUNCTION__);
    SPublicMsg *msg = new SPublicMsg;
    queue_.PutQ(msg, 3);
}
void TestThreadGroup::OnPrivateMsg() {
    BS_XLOG(XLOG_DEBUG,"TestThreadGroup::%s\n", __FUNCTION__);
    std::vector<TestTimerThread *>::iterator itr;
    for (itr = threads_.begin(); itr != threads_.end(); ++itr) {
        SPrivateMsg *msg = new SPrivateMsg;
        (*itr)->PutQ(msg);
    }
}
