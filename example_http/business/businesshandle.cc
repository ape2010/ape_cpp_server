#include "businesshandle.h"
#include "Serv.h"
#include <ape/loghelper.h>
#include <ape/urlcode.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <boost/shared_ptr.hpp>

namespace ape {
namespace testhttp {

typedef struct stThriftContext : public ape::message::SContext {
    ape::message::SHttpMessage *httpmsg;
    stThriftContext(ape::message::SHttpMessage *msg) : httpmsg(msg) {}
    virtual ~stThriftContext() {}
}SThriftContext;

CBusinessHandle::CBusinessHandle() : smseqid_(0) {
}
void CBusinessHandle::StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner) {
    threadid_ = threadid;
    service_ = service;
    timerowner_ = owner;
}
void CBusinessHandle::OnEvent(int threadid, void *event) {
    ape::message::SEvent *e = (ape::message::SEvent *)event;
    BS_XLOG(XLOG_TRACE,"CBusinessHandle::%s, event_id[%d], threadid[%d]\n", __FUNCTION__, e->id, threadid);
    switch (e->id) {
      case ape::message::SHttpMessage::ID: {
        ape::message::SHttpMessage * msg = (ape::message::SHttpMessage *)e;
        if (!msg->IsReply()) {
            DealHttpRequest(msg);
        }
        break;
      }
      case ape::message::SThriftMessage::ID: {
        ape::message::SThriftMessage * msg = (ape::message::SThriftMessage *)e;
        if (msg->IsReply()) {
            DealThriftResponse(msg);
        }
        delete e;
        break;
      }
      default: {
        delete e;
        break;
      }
    }
}
void CBusinessHandle::DealThriftResponse(ape::message::SThriftMessage *message) {
    BS_XLOG(XLOG_TRACE,"CBusinessHandle::%s, \n%s\n", __FUNCTION__, message->NoticeInfo().c_str());

    boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf(
        new apache::thrift::transport::TMemoryBuffer((uint8_t*)(message->body.c_str()), message->body.length()));
    boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol> pro(new apache::thrift::protocol::TBinaryProtocol(buf));
    teststudent::Serv_QueryByNo_result result;
    result.read(pro.get());

    teststudent::Student &stu = result.success;
    BS_XLOG(XLOG_DEBUG,"BusinessThread::%s, result.__isset.success[%d], no[%s], name[%s], age[%d]\n", __FUNCTION__,
        result.__isset.success, stu.no.c_str(), stu.name.c_str(), stu.age);

    SThriftContext *ctx = (SThriftContext *)(message->ctx);
    char szstu[1024] = {0};
    snprintf(szstu, 1023, "{\"return_code\":0, \"return_message\":\"success\",\"data\":{\"no\":\"%s\",\"name\":\"%s\",\"age\":\"%d\"}}",
        stu.no.c_str(), stu.name.c_str(), stu.age);
    DoSendHttpResponse(ctx->httpmsg, 200, szstu);
    delete ctx;
}
void CBusinessHandle::DealHttpRequest(ape::message::SHttpMessage *message) {
    message->Dump();
    if (0 == message->url.compare("/testthrift")) {
        DoTestThriftRequest(message);
        return;
    }
    DoSendHttpResponse(message, 200, "{\"return_code\":0, \"return_message\":\"success\"}");
}
void CBusinessHandle::DoSendHttpResponse(ape::message::SHttpMessage *request, int code, const std::string &body) {
    ape::message::SHttpMessage *response = new ape::message::SHttpMessage;
    response->SetReply(code);
    response->requestno = request->requestno;
    response->keepalive = request->keepalive;
    response->httpversion = request->httpversion;
    response->body = body;
    service_->DoSendBack(request->connid, response);
    delete request;
}
void CBusinessHandle::DoTestThriftRequest(ape::message::SHttpMessage *message) {
    teststudent::Serv_QueryByNo_args args;
    args.no = "0743111245";

    boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf(new apache::thrift::transport::TMemoryBuffer(1024));
    boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol> pro(new apache::thrift::protocol::TBinaryProtocol(buf));
    buf->open();
    args.write(pro.get());

    ape::message::SThriftMessage *msg = new ape::message::SThriftMessage;
    msg->method = "QueryByNo";
    msg->seqid = smseqid_++;
    msg->body = buf->getBufferAsString();
    msg->ctx = new SThriftContext(message);

    std::string addr = "thrift://127.0.0.1:10012";
    service_->DoConnect(addr, addr, true, 5000);
    service_->DoSendTo(addr, msg);
}
void CBusinessHandle::Dump() {
}
}
}
