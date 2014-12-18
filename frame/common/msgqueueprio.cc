#include "msgqueueprio.h"
#include "loghelper.h"
namespace ape {
namespace common {
MsgQueuePrio::MsgQueuePrio(int num_highqueuesize,int num_mediumqueuesize,int num_lowqueuesize):waitinggetthreadnum_(0) {
    queue_ = (MsgQueue *)malloc(MSG_PRIO_MAX * sizeof(MsgQueue));
    new(&queue_[MSG_HIGH]) MsgQueue(num_highqueuesize);
    new(&queue_[MSG_MEDIUM]) MsgQueue(num_mediumqueuesize);
    new(&queue_[MSG_LOW]) MsgQueue(num_lowqueuesize);
}
MsgQueuePrio::~MsgQueuePrio() {
    queue_[MSG_HIGH].~MsgQueue();
    queue_[MSG_MEDIUM].~MsgQueue();
    queue_[MSG_LOW].~MsgQueue();
    free(queue_);
}
bool MsgQueuePrio::PutQ(void *pdata, int num_seconds, EMsgPriority en_prio) {
    MsgQueue *p_queue = &(queue_[en_prio]);
    boost::unique_lock<boost::mutex> lock(m_mut);
    while (p_queue->IsFull()) {
        if (num_seconds == 0) {
            p_queue->waitingputthreadnum++;
            p_queue->cond_put.wait(lock);
            p_queue->waitingputthreadnum--;
        } else {
            p_queue->waitingputthreadnum++;
            if (p_queue->cond_put.timed_wait<boost::posix_time::seconds>(lock,boost::posix_time::seconds(num_seconds))==false) {
                p_queue->waitingputthreadnum--;
                return false;
            }
            p_queue->waitingputthreadnum--;
        }
    }
    p_queue->ppbuf[p_queue->putloc++] = pdata;
    ++(p_queue->datasize);
    if (p_queue->putloc == p_queue->capacity) {
        p_queue->putloc=0;
    }
    if(waitinggetthreadnum_) {
        cond_get_.notify_one();
    }
    return true;
}
void *MsgQueuePrio::GetQ(int num_seconds) {
    void *pdata = NULL;
    MsgQueue *p_queue = NULL;
    boost::unique_lock<boost::mutex> lock(m_mut);
    while(p_queue == NULL) {
        if (!queue_[MSG_HIGH].IsEmpty()) {
            p_queue = &(queue_[MSG_HIGH]);
        }
        else if(!queue_[MSG_MEDIUM].IsEmpty()) {
            p_queue = &(queue_[MSG_MEDIUM]);
        }
        else if(!queue_[MSG_LOW].IsEmpty()) {
            p_queue = &(queue_[MSG_LOW]);
        }
        if (p_queue != NULL) {
            pdata = p_queue->ppbuf[p_queue->getloc++];
            --(p_queue->datasize);
            if (p_queue->getloc == p_queue->capacity) {
                p_queue->getloc = 0;
            }
            if (p_queue->waitingputthreadnum)  {
                p_queue->cond_put.notify_one();
            }
            return pdata;
        } else {
            if (num_seconds == 0) {
                waitinggetthreadnum_++;
                cond_get_.wait(lock);
                waitinggetthreadnum_--;
            } else {
                waitinggetthreadnum_++;
                if(cond_get_.timed_wait<boost::posix_time::seconds>(lock,boost::posix_time::seconds(num_seconds)) == false) {
                    waitinggetthreadnum_--;
                    return NULL;
                }
                waitinggetthreadnum_--;
            }
        }
    }
    return NULL;
}
int MsgQueuePrio::GetCapacity(EMsgPriority en_prio) {
    return queue_[en_prio].capacity;
}
int MsgQueuePrio::GetUsed(EMsgPriority en_prio) {
    return queue_[en_prio].datasize;
}
bool MsgQueuePrio::IsEmpty() {
    return queue_[MSG_HIGH].IsEmpty() && queue_[MSG_MEDIUM].IsEmpty() && queue_[MSG_LOW].IsEmpty();
}

void MsgQueuePrio::Dump() {
    BS_XLOG(XLOG_DEBUG, "==========MsgQueuePrio::Dump==========\n");
    boost::unique_lock<boost::mutex> lock(m_mut);
    BS_XLOG(XLOG_DEBUG, " high_queue: capacity[%d], datasize[%d], putloc[%d], getloc[%d]\n",
        queue_[MSG_HIGH].capacity, queue_[MSG_HIGH].datasize, queue_[MSG_HIGH].putloc,
        queue_[MSG_HIGH].getloc);
    BS_XLOG(XLOG_DEBUG, " middle_queue: capacity[%d], datasize[%d], putloc[%d], getloc[%d]\n",
        queue_[MSG_MEDIUM].capacity, queue_[MSG_MEDIUM].datasize, queue_[MSG_MEDIUM].putloc,
        queue_[MSG_MEDIUM].getloc);
    BS_XLOG(XLOG_DEBUG, " low_queue: capacity[%d], datasize[%d], putloc[%d], getloc[%d]\n",
        queue_[MSG_LOW].capacity, queue_[MSG_LOW].datasize, queue_[MSG_LOW].putloc,
        queue_[MSG_LOW].getloc);
}
}
}
