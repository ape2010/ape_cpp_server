#ifndef _TEST_THREAD_H_
#define _TEST_THREAD_H_
#include "msgtimerthread.h"
#include "threadtimer.h"
using namespace ape::common;

typedef struct stMsg { 
    int id; 
    stMsg(int i):id(i) {} 
    virtual ~stMsg(){} 
} SMsg;
typedef struct stPrivateMsg : SMsg { 
    stPrivateMsg():SMsg(0){} 
    virtual ~stPrivateMsg(){} 
} SPrivateMsg;
typedef struct stPublicMsg : SMsg { 
    stPublicMsg():SMsg(1){} 
    virtual ~stPublicMsg(){}  
} SPublicMsg;

class TestTimerThread : public MsgTimerThread{
 public:
    TestTimerThread(MsgQueuePrio *queue, int id);
    virtual void StartInThread();
    virtual void StopInThread();
    virtual void Deal(void *pdata);
 private:
    void DoPublicMsg(SMsg *msg);
    void DoPrivateMsg(SMsg *msg);
    void SelfCheck();
 public:
    int id_;
    CThreadTimer timerselfcheck_;
};
class TestThreadGroup {
 public:
    static TestThreadGroup *GetInstance();
    static void Release();
    ~TestThreadGroup();
    void Start(int threads = 5);
    void Stop();
    void OnPublicMsg();
    void OnPrivateMsg();
 private:
    TestThreadGroup();
 private:
    static TestThreadGroup *sminstance_;
    MsgQueuePrio queue_;
    std::vector<TestTimerThread *> threads_;
};


#endif
