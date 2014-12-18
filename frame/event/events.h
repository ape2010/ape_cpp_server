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
    int id;
    stEvent(int i = -1) : id(i) {}
    virtual ~stEvent(){}
    virtual std::string NoticeInfo() {return "";};
    virtual void Dump() {};
}SEvent;

template <int tid, class BaseClass = SEvent>
struct SEventType : public BaseClass {
    enum {ID=tid};
    SEventType() : BaseClass(tid) {}
    virtual ~SEventType() {}
};

typedef struct stNetEvent : public SEvent {
    char ip[16];
    unsigned int port;
    unsigned int connid;
    struct timeval time;
    std::string session_name;
    stNetEvent(int id=-1) : SEvent(id), port(0), connid(0) {memset(ip, 0, 16);}
    virtual ~stNetEvent(){}
    virtual std::string NoticeInfo() {
        char sz[32] = {0};
        snprintf(sz, 31, "%s:%u", ip, port);
        return std::string(sz);
    }
    virtual std::string BriefInfo() {return NoticeInfo();}
    virtual void Init(const char *iip, unsigned int pt) {
        memcpy(ip, iip, strlen(iip));
        port = pt;
        gettimeofday(&time, NULL);
    }
    virtual void Dump(){}
}SNetEvent;

typedef struct stConnectedEvent : public SEventType<101, SNetEvent> {
    virtual ~stConnectedEvent(){}
}SConnectedEvent;
typedef struct stPeerCloseEvent : public SEventType<102, SNetEvent> {
    virtual ~stPeerCloseEvent(){}
}SPeerCloseEvent;


typedef struct stContext {
  uint64_t token;
  virtual ~stContext() {}
}SContext;

typedef struct stNetMessage : public SNetEvent {
    typedef enum  {E_Request = 0, E_Response, E_One_Way}SMessageDirection;
    SMessageDirection direction;
    bool isheartbeat;
    int code;
    SContext *ctx;
    void SetReply(int n = 0) {direction = E_Response; code = n;}
    bool IsReply() {return direction == E_Response;}
    virtual unsigned int GetSequenceId() {return 0;};
    virtual bool IsOk() {return code == 0;}
    stNetMessage(int id=-1) : SNetEvent(id), direction(E_Request), isheartbeat(false), code(-1), ctx(NULL){}
    virtual ~stNetMessage(){ }
    virtual std::string BriefInfo() {return "";}
}SNetMessage;

}
}

#endif
