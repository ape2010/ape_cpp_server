#include "cohthread.h"
#include <boost/bind.hpp>
#include <boost/date_time.hpp>
#include <boost/algorithm/string.hpp>
#include <dlfcn.h>
#include <ape/loghelper.h>

namespace ape {
namespace testhttp {
static void ParseUrl(const std::string &url, CohThread::SUrlInfo *info) {
    char szhost[128]={0};
    char szpath[256]={0};

    sscanf(url.c_str(), "%*[HTTPhttp]://%[^/]%256s", szhost, szpath);
    info->addr = std::string("http://") + szhost;
    info->name = std::string("monitor_") + szhost;
    info->path = szpath;
}

CohThread * CohThread::sm_instance_ = NULL;
CohThread * CohThread::GetInstance() {
    if(sm_instance_ == NULL)
        sm_instance_ = new CohThread;
    return sm_instance_;
}
CohThread::CohThread() :service_(NULL) {
    mapcmd_["/help"] = SFuncInfo(FUNC_HELP, "Get Cmd List");
    funcmapping_[FUNC_HELP] = &CohThread::DoHelpInfo;
}
CohThread::~CohThread() {
}
int CohThread::StartServer(const std::string &addr) {
    CohHandleFactory factory;
    holder_.Start(&factory, 0);

    std::vector<std::string> addrs;
    addrs.push_back(addr);
    return holder_.StartServer(addrs);
}
void CohThread::Stop() {
    holder_.Stop();
}
void CohThread::InitUrl(const std::string &errorrurl, const std::string &actionurl) {
    ParseUrl(errorrurl, &error_url_);
    ParseUrl(actionurl, &action_url);
}
void CohThread::OnErrorReport(const std::string &info) {
    ape::message::SHttpMessage *msg = new ape::message::SHttpMessage;
    msg->httpversion = ape::message::SHttpMessage::HTTP_1_0;
    msg->method = "POST";
    msg->url = error_url_.path;
    msg->keepalive = false;
    msg->body = info;
    if (service_) {
        service_->OnConnect(error_url_.name, error_url_.addr, true, 5000);
        service_->OnSendTo(error_url_.name, msg);
    }
}
void CohThread::OnActionReport(const std::string &info) {
    ape::message::SHttpMessage *msg = new ape::message::SHttpMessage;
    msg->httpversion = ape::message::SHttpMessage::HTTP_1_0;
    msg->method = "POST";
    msg->url = action_url.path;
    msg->keepalive = false;
    msg->body = info;
    if (service_) {
        service_->OnConnect(action_url.name, action_url.addr);
        service_->OnSendTo(action_url.name, msg);
    }
}
void CohThread::StartInThread(int threadid, ape::net::CNetService *service, ape::common::CTimerManager *owner) {
    service_ = service;
}
void CohThread::OnEvent(int threadid, void *event) {
    ape::message::SEvent *e = (ape::message::SEvent *)event;
    BS_XLOG(XLOG_TRACE,"CohThread::%s, type[%d], threadid[%d]\n", __FUNCTION__, e->id, threadid);
    switch (e->id) {
      case ape::message::SHttpMessage::ID: {
        ape::message::SHttpMessage *message = (ape::message::SHttpMessage *)e;
        message->Dump();
        if (message->type == ape::message::SNetMessage::E_Request) {
            DealCohCmd(message);
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
void CohThread::DealCohCmd(ape::message::SHttpMessage *message) {
    ape::message::SHttpMessage *response = new ape::message::SHttpMessage;
    transform(message->url.begin(), message->url.end(), message->url.begin(), ::tolower);
    std::map<std::string, SFuncInfo>::iterator itr = mapcmd_.find(message->url);
    if (itr != mapcmd_.end()) {
        response->body = (this->*(funcmapping_[itr->second.type]))(message);
    } else {
        response->body = "Bad Command; Try /Help";
    }

    response->code = 200;
    response->SetReply(200);
    response->requestno = message->requestno;
    response->keepalive = message->keepalive;
    response->httpversion = message->httpversion;
    service_->DoSendBack(message->connid, response);
}
std::string CohThread::DoHelpInfo(ape::message::SHttpMessage *message) {
    std::string str = "<html><head><title>Help</title></head><body><table border='1' align='center'>";
    str.append("<tr><td>command</td><td>description</td></tr>");
    std::map<std::string, SFuncInfo>::iterator itr = mapcmd_.begin();
    for(; itr != mapcmd_.end(); ++itr) {
        str.append("<tr><td>");
        str.append(itr->first);
        str.append("</td><td>");
        str.append(itr->second.desc);
        str.append("</td></tr>");
    }
    str.append("</table></body></html>");
    return str;
}
}
}
