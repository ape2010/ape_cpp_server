#ifndef _APE_COMMON_EVENTS_H_
#define _APE_COMMON_EVENTS_H_

#include <sys/time.h>
#include <string>
#include <cstring>
#include <stdio.h>
#include <map>
#include <vector>
#include <boost/unordered_map.hpp>

namespace ape {
namespace message {
typedef enum {
    HTTP_MESSAGE_EVENT = 0,
    THRIFT_MESSAGE_EVENT,

    EVENT_ALL
}EEventType;

typedef struct stEvent {
    EEventType id;
    stEvent(EEventType i) : id(i) {}
    virtual ~stEvent(){}
    virtual std::string NoticeInfo() = 0;
    virtual void Dump() = 0;
}SEvent;
typedef struct stContext {
  uint64_t token;
  virtual ~stContext() {}
}SContext;
typedef struct stNetEvent : public SEvent {
    char ip[16];
    unsigned int port;
    unsigned int connid;
    struct timeval time;
    stNetEvent(EEventType i) : SEvent(i), port(0) {memset(ip, 0, 16);}
    virtual ~stNetEvent(){}
    virtual std::string NoticeInfo();
    virtual void Init(const char *iip, unsigned int pt);
    virtual void Dump(){}
}SNetEvent;
typedef struct stNetMessage : public SNetEvent {
    typedef enum  {E_Request = 0, E_Response}SMessageType;
    SMessageType type;
    bool isheartbeat;
    int code;
    SContext *ctx;
    virtual unsigned int GetSequenceId() = 0;
    void SetReply(int n = 0){type = E_Response; code = n;}
    bool IsReply() {return type == E_Response;}
    virtual bool IsOk() {return code == 0;}
    stNetMessage(EEventType i) : SNetEvent(i), type(E_Request), isheartbeat(false), code(-1), ctx(NULL){}
    virtual ~stNetMessage(){ }
}SNetMessage;

typedef struct stThriftMessage : public SNetMessage {
    unsigned int seqid;
    std::string method;
    std::string body;
    stThriftMessage() : SNetMessage(THRIFT_MESSAGE_EVENT), seqid(0) {}
    virtual ~stThriftMessage();
    virtual unsigned int GetSequenceId(){return seqid;}
    virtual std::string NoticeInfo();
    virtual void Dump();
}SThriftMessage;

typedef struct stHttpMessage : public SNetMessage {
    typedef struct stSCookie {
        std::string value;
        std::string path;
        std::string domain;
        time_t expires;
        stSCookie() :expires (0) {}
        stSCookie(const std::string &v, const std::string &p, const std::string &d, time_t e = 0) :
            value(v), path(p), domain(d), expires(e) {}
    }SCookie;
    typedef enum { HTTP_1_0 = 0, HTTP_1_1 = 1} EHttpVersion;

    bool keepalive;
    unsigned int requestno;
    EHttpVersion httpversion;
    std::string method;
    std::string url;
    std::string xforwordip;
    std::string body;
    boost::unordered_map<std::string, std::string> headers;
    boost::unordered_map<std::string, SCookie> cookies;

    stHttpMessage() : SNetMessage(HTTP_MESSAGE_EVENT), keepalive(false), requestno(0), httpversion(HTTP_1_0) {}
    virtual ~stHttpMessage(){}
    virtual unsigned int GetSequenceId(){return 0;}
    virtual bool IsOk() {return code == 200;}
    virtual std::string NoticeInfo();
    virtual void Dump();

    void AddHeader(const std::string &key, const std::string &value);
    void SetCookie(const std::string &key, const std::string &value, const std::string &path, const std::string &domain, time_t e = 0);
}SHttpMessage;

std::string GetBinaryDumpInfo(const char *buf, int len, int indent = 4);
}
}

#endif
