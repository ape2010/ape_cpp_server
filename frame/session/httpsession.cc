#include "httpsession.h"
#include "sessionmanager.h"
#include "loghelper.h"

namespace ape{
namespace net{
HttpSessionFactory HttpSessionFactory::http_session_factory_;

CHttpSession::~CHttpSession() {
    while( !request_deque_.empty()) {
        if (NULL != request_deque_.begin()->response) {
            delete request_deque_.begin()->response;
        }
        request_deque_.pop_front();
    }
}
void CHttpSession::OnRead(ape::message::SNetMessage *msg) {
    if (msg->type == ape::message::SNetMessage::E_Request) {
        ((ape::message::SHttpMessage *)msg)->requestno = ++dwseq;
        request_deque_.push_back(SResponseItem(((ape::message::SHttpMessage *)msg)->requestno));
    }
    CSession::OnRead(msg);
}
void CHttpSession::DoSendBack(void *para, bool close) {
    ape::message::SHttpMessage *message = (ape::message::SHttpMessage *)para;
    BS_XLOG(XLOG_DEBUG,"CHttpSession::%s, no[%d]\n", __FUNCTION__, message->requestno);
    std::deque<SResponseItem>::iterator itr = request_deque_.begin();
    if (itr->no == message->requestno) {
        CSession::DoSendBack(message, !message->keepalive);
        request_deque_.pop_front();
        while( !request_deque_.empty() && NULL != request_deque_.begin()->response) {
            message = (ape::message::SHttpMessage *)(request_deque_.begin()->response);
            CSession::DoSendBack(message, !message->keepalive);
            request_deque_.pop_front();
        }
        return;
    } else {
        for(++itr; itr != request_deque_.end(); ++itr) {
            if(itr->no == message->requestno) {
                itr->response = message;
                return;
            }
        }
    }
    BS_XLOG(XLOG_WARNING,"CHttpSession::%s, no request to response, no[%d]\n", __FUNCTION__, message->requestno);
    delete message;
}
void CHttpSession::Dump() {
    CSession::Dump();
}
}
}
