#include "events.h"
#include "loghelper.h"

namespace ape {
namespace message {
static inline char GetVisibleAscii(char c) {
    return (c >= 33 && c <= 126) ? c : '.';
}
std::string SNetEvent::NoticeInfo() {
    char sz[32] = {0};
    snprintf(sz, 31, "%s:%u", ip, port);
    return std::string(sz);
}
void SNetEvent::Init(const char *iip, unsigned int pt) {
    memcpy(ip, iip, strlen(iip));
    port = pt;
    gettimeofday(&time, NULL);
}
SThriftMessage::~stThriftMessage() {
}
std::string SThriftMessage::NoticeInfo() {
    const char *indent = "      ";
    std::string info;
    info.append(indent);
    info.append(method);
    char sz[64] = {0};
    snprintf(sz, 63, ", seq[%d], code[%d]", seqid, code);
    info.append(sz);
    info.append(1,'\n');
    
    if (!body.empty())
        info.append(GetBinaryDumpInfo(body.c_str(), body.length(), 6));
    return info;
}
void SThriftMessage::Dump() {
    BS_XLOG(XLOG_DEBUG,"SThriftMessage::%s\n%s\n",__FUNCTION__, NoticeInfo().c_str());
}

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

std::string GetBinaryDumpInfo(const char *buf, int len, int indent) {
    std::string info;
    std::string strindent(indent, ' ');
    
    int line = len >> 3;
    int last = (len & 0x7);
    int i = 0;
    for (i = 0; i < line; ++i) {
        char szbuf[128] = {0};
        const unsigned char * base = (const unsigned char *)(buf + (i << 3));
        sprintf(szbuf,"%s[%2d]  %02x %02x %02x %02x %02x %02x %02x %02x    %c %c %c %c %c %c %c %c\n",
            strindent.c_str(), i,*(base),*(base+1),*(base+2),*(base+3),*(base+4),*(base+5),*(base+6),*(base+7),
            GetVisibleAscii(*(base)), GetVisibleAscii(*(base+1)), GetVisibleAscii(*(base+2)), GetVisibleAscii(*(base+3)),
            GetVisibleAscii(*(base+4)), GetVisibleAscii(*(base+5)), GetVisibleAscii(*(base+6)), GetVisibleAscii(*(base+7)));
        info.append(szbuf);
    }
    if (last > 0) {
        const unsigned char * base = (const unsigned char *)(buf + (i << 3));
        char szhex[32] = {0};
        char szascii[32] = {0};
        for(int j = 0; j < last; ++j) {
            sprintf(szhex + j * 3, " %02x", *(base + j));
            sprintf(szascii + j * 2, " %c", GetVisibleAscii(*(base + j)));
        }
        char szbuf[128] = {0};
        sprintf(szbuf, "%s[%2d] %s", strindent.c_str(), i, szhex);
        info.append(szbuf);
        info.append((8 - last)*3 + 3, ' ');
        info.append(szascii);
        info.append(1,'\n');
    }
    return info;
}
}
}