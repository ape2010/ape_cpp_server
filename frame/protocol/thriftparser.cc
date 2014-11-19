#include "thriftparser.h"
#include "loghelper.h"
#include "urlcode.h"
#include <stdlib.h>
#include <algorithm>
#include <arpa/inet.h>

namespace ape{
namespace protocol{
ThriftParserFactory ThriftParserFactory::thrift_parser_factory_;

typedef struct stThriftHead {
    uint32_t size;
    uint16_t version;
    uint16_t type;
    uint32_t methodlen;
}SThriftHead;
enum ThriftMessageType {
  T_CALL       = 1,
  T_REPLY      = 2,
  T_EXCEPTION  = 3,
  T_ONEWAY     = 4
};
static const int16_t VERSION_1 = (int16_t)0x8001;

CThriftParser::CThriftParser() {
}
const char *CThriftParser::Decode(const char *buf, int len, ape::message::SNetMessage *msg) {
    uint32_t skip = sizeof(SThriftHead);
    if (len < skip) {
        return buf;
    }
    SThriftHead head = *((SThriftHead *)buf);
    head.size = ntohl(head.size);
    head.version = ntohs(head.version);
    head.type = ntohs(head.type);
    head.methodlen = ntohl(head.methodlen);
    if (head.size + 4 > len) {
        return buf;
    }

    ape::message::SThriftMessage *message = (ape::message::SThriftMessage *)msg;
    if (head.type == T_CALL || head.type == T_ONEWAY) {
        message->type = ape::message::SNetMessage::E_Request;
    } else if (head.type == T_REPLY){
        message->type = ape::message::SNetMessage::E_Response;
    } else {
        BS_XLOG(XLOG_WARNING,"CThriftParser::%s, invalidate type[%d], method[%s]\n", __FUNCTION__, head.type, message->method.c_str());
        return NULL;
    }
    if (skip + head.methodlen > len) {
        return NULL;
    }
    message->method.assign(buf + skip, head.methodlen);
    if (17 == head.methodlen && 0 == strncasecmp(message->method.c_str(), "AllsparkHeartBeat", 17)) {
        message->isheartbeat = true;
    }
    skip += head.methodlen;
    message->seqid = *((uint32_t *)(buf + skip));
    skip += 4;
    uint32_t bodylen = head.size + 4 - skip;
    message->body.assign(buf + skip, bodylen);
    return buf + skip + bodylen;
}
int CThriftParser::Encode(const ape::message::SNetMessage *msg, ape::common::CBuffer *out) {
    ape::message::SThriftMessage *message = (ape::message::SThriftMessage *)msg;
    SThriftHead head;
    head.size = sizeof(SThriftHead) + message->method.length() + message->body.length(); // -4 + 4
    head.size = htonl(head.size);
    head.version = htons(VERSION_1);
    head.type = htons(message->IsReply() ? T_REPLY : T_CALL);
    head.methodlen = htonl(message->method.length());
    out->append((char *)(&head), sizeof(SThriftHead));
    out->append(message->method.c_str(), message->method.length());
    uint32_t seqid = htonl(message->seqid);
    out->append((char *)&seqid, 4);
    out->append(message->body.c_str(), message->body.length());
    return 0;
}

ape::message::SNetMessage *CThriftParser::CreateHeartBeatMessage(ape::message::SNetMessage::SMessageType type) {
    ape::message::SThriftMessage *msg = new ape::message::SThriftMessage;
    msg->method = "AllsparkHeartBeat";
    msg->seqid = 0;
    msg->type = type;
    return msg;
}
}
}

