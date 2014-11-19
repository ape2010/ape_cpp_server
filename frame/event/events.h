#ifndef _APE_COMMON_EVENTS_H_
#define _APE_COMMON_EVENTS_H_

#include <sys/time.h>
#include <string>
#include <map>
#include <cstring>
#include <stdio.h>
#include <vector>
#include <boost/unordered_map.hpp>
#include "protocol.h"

namespace ape {
namespace message {

typedef struct stEvent {
    virtual ~stEvent(){}
    virtual std::string NoticeInfo() = 0;
    virtual void Dump() = 0;
}SEvent;

template <int id, class BaseClass = SEvent>
struct SEventType : public BaseClass {
    enum {ID=id};
    virtual ~SEventType() {}
};

typedef struct stNetEvent : public SEvent {
    char ip[16];
    unsigned int port;
    unsigned int connid;
    struct timeval time;
    stNetEvent() : port(0) {memset(ip, 0, 16);}
    virtual ~stNetEvent(){}
    virtual std::string NoticeInfo() {
        char sz[32] = {0};
        snprintf(sz, 31, "%s:%u", ip, port);
        return std::string(sz);
    }
    virtual void Init(const char *iip, unsigned int pt) {
        memcpy(ip, iip, strlen(iip));
        port = pt;
        gettimeofday(&time, NULL);
    }
    virtual void Dump(){}
}SNetEvent;

typedef struct stContext {
  uint64_t token;
  virtual ~stContext() {}
}SContext;

typedef struct stNetMessage : public SNetEvent {
    typedef enum  {E_Request = 0, E_Response}SMessageType;
    SMessageType type;
    bool isheartbeat;
    int code;
    SContext *ctx;
    virtual unsigned int GetSequenceId() = 0;
    void SetReply(int n = 0) {type = E_Response; code = n;}
    bool IsReply() {return type == E_Response;}
    virtual bool IsOk() {return code == 0;}
    stNetMessage() : type(E_Request), isheartbeat(false), code(-1), ctx(NULL){}
    virtual ~stNetMessage(){ }
}SNetMessage;

}
}

#endif
