#ifndef _APE_MESSAGE_THRIFT_MESSAGE_H_
#define _APE_MESSAGE_THRIFT_MESSAGE_H_

#include "events.h"

namespace ape {
namespace message {

typedef struct stThriftMessage : public SEventType<ape::protocol::E_PROTOCOL_THRIFT, SNetMessage> {
    unsigned int seqid;
    std::string method;
    std::string body;
    stThriftMessage() : seqid(0) {}
    virtual ~stThriftMessage();
    virtual unsigned int GetSequenceId(){return seqid;}
    virtual std::string NoticeInfo();
    virtual std::string BriefInfo();
    virtual void Dump();
}SThriftMessage;


}
}

#endif
