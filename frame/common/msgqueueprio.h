#ifndef APE_COMMON_MSG_QUEUE_PIRO_H_
#define APE_COMMON_MSG_QUEUE_PIRO_H_

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

namespace ape {
namespace common {
#define DEFAULT_MEDIUMQUEUESIZE 1024*1024

class MsgQueuePrio {
 public:
    typedef enum emMsgPriority {
          MSG_HIGH=0,
          MSG_MEDIUM=1,
          MSG_LOW=2,
          MSG_PRIO_MAX
    }EMsgPriority;
    typedef struct stMsgQueue {
        boost::condition_variable cond_put;
        void **ppbuf;   
        int capacity;     
        int datasize;   
        int putloc;     
        int getloc;      
        int waitingputthreadnum;    
        
        stMsgQueue(int size):capacity(size),datasize(0),putloc(0),getloc(0),waitingputthreadnum(0){ ppbuf = new void *[size]; }
        ~stMsgQueue(){ delete []ppbuf; }
        bool IsFull() { return capacity == datasize;}
        bool IsEmpty() { return datasize <= 0;}
    }MsgQueue;
 public:
    MsgQueuePrio(int num_highqueuesize=512,int num_mediumqueuesize=DEFAULT_MEDIUMQUEUESIZE,int num_lowqueuesize=512);
    ~MsgQueuePrio();
    bool PutQ(void *pdata, int num_seconds = 0, EMsgPriority en_prio = MSG_MEDIUM);
    void *GetQ(int num_seconds = 1);
    int GetCapacity(EMsgPriority en_prio = MSG_MEDIUM);
    int GetUsed(EMsgPriority en_prio = MSG_MEDIUM);
    void Dump();
 private:
    MsgQueue *queue_;
    
    int waitinggetthreadnum_;
    boost::condition_variable cond_get_;
    boost::mutex m_mut;
};
}
}
#endif
