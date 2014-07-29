#include "businesshandle.h"
#include "Serv.h"
#include <ape/loghelper.h>
#include <ape/urlcode.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TBufferTransports.h>
#include <boost/shared_ptr.hpp>

namespace ape {
namespace testthrift {
CBusinessHandle::CBusinessHandle() {
}
void CBusinessHandle::StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner) {
    threadid_ = threadid;
    service_ = service;
    timerowner_ = owner;
}
void CBusinessHandle::OnEvent(int threadid, void *event) {
    ape::message::SEvent *e = (ape::message::SEvent *)event;
    BS_XLOG(XLOG_TRACE,"CBusinessHandle::%s, type[%d], threadid[%d]\n", __FUNCTION__, e->id, threadid);
    switch (e->id) {
      case ape::message::THRIFT_MESSAGE_EVENT: {
        ape::message::SThriftMessage * msg = (ape::message::SThriftMessage *)e;
        if (!msg->IsReply()) {
            DealThriftRequest(msg);
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
void CBusinessHandle::DealThriftRequest(ape::message::SThriftMessage *message) {
    BS_XLOG(XLOG_TRACE,"CBusinessHandle::%s, \n%s\n", __FUNCTION__, message->NoticeInfo().c_str());

    boost::shared_ptr<apache::thrift::transport::TMemoryBuffer> buf(new apache::thrift::transport::TMemoryBuffer((uint8_t*)(message->body.c_str()), message->body.length()));
    boost::shared_ptr<apache::thrift::protocol::TBinaryProtocol> pro(new apache::thrift::protocol::TBinaryProtocol(buf));
    teststudent::Serv_QueryByNo_args args;
    args.read(pro.get());
    BS_XLOG(XLOG_TRACE,"CBusinessHandle::%s, no[%s]\n", __FUNCTION__, args.no.c_str());

    teststudent::Serv_QueryByNo_result result;
    result.success.no = "0743111245";
    result.success.name = "test_student_xiaozhai";
    result.success.age = 24;
    result.__isset.success = true;
    
    buf.reset(new apache::thrift::transport::TMemoryBuffer(10240));
    buf->open();
    pro.reset(new apache::thrift::protocol::TBinaryProtocol(buf));
    result.write(pro.get());
    
    ape::message::SThriftMessage *resmsg = new ape::message::SThriftMessage;
    resmsg->method = "QueryByNo";
    resmsg->seqid = message->seqid;
    resmsg->body = buf->getBufferAsString();
    resmsg->SetReply();
    service_->DoSendBack(message->connid, resmsg);
}
void CBusinessHandle::Dump() {
}
}
}
