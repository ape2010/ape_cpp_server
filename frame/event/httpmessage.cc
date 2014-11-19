#include "httpmessage.h"
#include "loghelper.h"

namespace ape {
namespace message {

void SHttpMessage::AddHeader(const std::string &key, const std::string &value) {
    headers[key] = value;
}
void SHttpMessage::SetCookie(const std::string &key, const std::string &value,
    const std::string &path, const std::string &domain, time_t e)
{
    cookies[key] = SCookie(value, path, domain, e);
}
std::string SHttpMessage::NoticeInfo() {
    std::string indent = "    ";
    std::string info = indent;
    if (type == SNetMessage::E_Request) {
        info += method + " " + url + " " + (httpversion == HTTP_1_1 ? "HTTP/1.1" : "HTTP/1.0") + "\r\n";
    } else {
        info.append(httpversion == HTTP_1_1 ? "HTTP/1.1 " : "HTTP/1.0 ");
        char szcode[32] = {0};
        snprintf(szcode, 31, "%d \r\n", code);
        info.append(szcode);
    }

    boost::unordered_map<std::string, std::string>::const_iterator itr = headers.begin();
    for (; itr != headers.end(); ++itr) {
        info += indent + itr->first + ": " + itr->second + "\r\n";
    }

    boost::unordered_map<std::string, SCookie>::const_iterator itrc = cookies.begin();
    for (; itrc != cookies.end(); ++itrc) {
        info += indent + "Cookie: " + itrc->first + "=" + itrc->second.value +
            ";path=" + itrc->second.path + "; domain=" + itrc->second.domain + "\r\n";
    }
    info += indent + body;
    return info;
}
void SHttpMessage::Dump() {
    BS_XLOG(XLOG_DEBUG,"SHttpMessage::%s, keepalive[%d], no[%d]\n%s\n",__FUNCTION__, keepalive, requestno, NoticeInfo().c_str());
}

}
}