#ifndef _APE_COMMON_MSG_TIMERTHREAD_H_
#define _APE_COMMON_MSG_TIMERTHREAD_H_
#include "msgqueueprio.h"
#include "timermanager.h"
#include <boost/thread.hpp>

namespace ape {
namespace common {
class MsgTimerThread : public CTimerManager {
 public:
    MsgTimerThread();
    MsgTimerThread(MsgQueuePrio *public_queue, bool self_priority = false);
    virtual ~MsgTimerThread();

    void Start();
    void Stop();
    bool IsRunning() {return is_running_;}
    bool PutQ(void *pdata, int num_seconds = 0, MsgQueuePrio::EMsgPriority en_prio = MsgQueuePrio::MSG_MEDIUM);
    virtual void StartInThread() {}
    virtual void StopInThread() {}
    virtual void Deal(void *pdata) {}
    virtual void Dump() {CTimerManager::Dump();}

 private:
    void Run();

 private:
    boost::thread thread_;
    bool self_priority_;
    volatile bool is_running_;
    MsgQueuePrio queue_;
    MsgQueuePrio *public_queue_;
};
}
}

#endif