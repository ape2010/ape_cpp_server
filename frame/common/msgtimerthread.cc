#include "msgtimerthread.h"
#include "loghelper.h"

namespace ape {
namespace common {
MsgTimerThread::MsgTimerThread():self_priority_(true), is_running_(false), public_queue_(NULL) {}
MsgTimerThread::MsgTimerThread(MsgQueuePrio *public_queue, bool self_priority) : self_priority_(self_priority), is_running_(false), public_queue_(public_queue) {}
MsgTimerThread::~MsgTimerThread() {}

void MsgTimerThread::Start() {
    thread_ = boost::thread(boost::bind(&MsgTimerThread::Run,this));
}
void MsgTimerThread::Stop() {
    is_running_ = false;
    //thread_.interrupt();
    thread_.join();
}
bool MsgTimerThread::PutQ(void *pdata, int num_seconds, MsgQueuePrio::EMsgPriority en_prio) {
    return queue_.PutQ(pdata, num_seconds, en_prio);
}
void MsgTimerThread::Run() {
    is_running_ = true;
    StartInThread();
    void *pdata = NULL;

    while (1) {
        if (self_priority_) {
            if(public_queue_ != NULL && !public_queue_->IsEmpty())
            {
                pdata = public_queue_->GetQ(1);
                if(pdata!=NULL)
                    Deal(pdata);
            }
            pdata = queue_.GetQ(1);
            if(pdata != NULL) {
                Deal(pdata);
            }
        } else {
            if(!queue_.IsEmpty())
            {
                pdata = queue_.GetQ(1);
                if(pdata!=NULL)
                    Deal(pdata);
            }
            if (public_queue_ != NULL) {
                pdata = public_queue_->GetQ(1);
                if (pdata) {
                    Deal(pdata);
                }
            }
        }
        if (!is_running_) {
            StopInThread();
            break;
        }
        DetectTimerList();
    }
}
}
}
