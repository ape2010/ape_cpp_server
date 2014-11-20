#include "httpparser.h"
#include "loghelper.h"
#include "urlcode.h"
#include <stdlib.h>
#include <algorithm>

namespace ape{
namespace protocol{

static inline std::string trim(const char *buf, int len, bool tolowercase = false) {
    const char *begin = buf;
    const char *end = buf + len - 1;
    for(; *begin == ' ' && begin < end; ++begin){}
    for(; *end == ' ' && end > begin; --end){}
    std::string ret;
    for (const char *p = begin; p <= end; ++p) {
        if (tolowercase && *p >= 'A' && *p <= 'Z') {
            ret.append(1, tolower(*p));
        } else {
            ret.append(1, *p);
        }
    }
    return ret;
}
static inline std::string httptime(time_t t) {
    struct tm gmt;
    gmtime_r(&t, &gmt);
    char szdate[128] = {0};
    strftime(szdate, 127, "%a, %d %b %Y %H:%M:%S %Z", &gmt);
    return std::string(szdate);
}
static inline time_t httptime(const char *buf) {
    struct tm gmt; //Fri, 23 May 2014 06:36:18 GMT
    memset(&gmt, 0, sizeof(gmt));
    if (strptime(buf, "%a, %d %b %Y %H:%M:%S %Z", &gmt) == NULL) {
        return 0;
    }
    time_t t = mktime(&gmt);
    return t > 0 ? t : 0;
}
static inline std::string GetStatus(int code) {
    switch(code) {
        case 200: return "200 OK";
        case 201: return "201 Created";
        case 202: return "202 Accepted";
        case 204: return "204 No Content";
        case 300: return "300 Multiple Choices";
        case 301: return "301 Moved Permanently";
        case 302: return "302 Moved Temporarily";
        case 304: return "304 Not Modified";
        case 400: return "400 Bad Request";
        case 401: return "401 Unauthorized";
        case 403: return "403 Forbidden";
        case 404: return "404 Not Found";
        case 405: return "405 Method Not Allowed";
        case 500: return "500 Internal Server Error";
        case 501: return "501 Not Implemented";
        case 502: return "502 Bad Gateway";
        case 503: return "503 Service Unavailable";
        default: {
            char szcode[8] = {0};
            snprintf(szcode, 7, "%d Unknow Code", code);
            return std::string(szcode);
        }
    }
}
std::map<std::string, std::string> CHttpParser::sm_default_header_;
std::map<std::string, std::string> CHttpParser::sm_default_request_header_;
CHttpParser::CHttpParser() : contentlen_(0), ischunked_(false) {
    if (sm_default_header_.empty()) {
        sm_default_header_["Server"] = "Ape Http Server 1.0";
        sm_default_header_["Cache-Control"] = "no-cache";
        sm_default_header_["Content-Type"] = "text/html; charset=utf-8";
    }
    if (sm_default_request_header_.empty()) {
        sm_default_request_header_["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
        sm_default_request_header_["Cache-Control"] = "no-cache";
        sm_default_request_header_["User-Agent"] = "ApeHttpClient";
        sm_default_request_header_["Accept-Language"] = "en-US,en;q=0.8,zh-CN;q=0.6,zh-TW;q=0.4";
    }
}
const char *CHttpParser::Decode(const char *buf, int len, ape::message::SNetMessage *msg) {
    if (len < 16) { //incomplete packet
        return buf;
    }
    ape::message::SHttpMessage *message = (ape::message::SHttpMessage *)msg;
    contentlen_ = 0;
    ischunked_ = false;
    const char *begin = buf;
    const char *end = begin;
    bool isresponse = false;
    if (0 == strncasecmp(buf, "GET", 3)) {
        begin += 3;
    } else if (0 == strncasecmp(buf, "POST", 4)) {
        begin += 4;
    } else if (0 == strncasecmp(buf, "HEAD", 4)) {
        begin += 4;
    } else if (0 == strncasecmp(buf, "TRACE", 5)) {
        begin += 5;
    } else if (0 == strncasecmp(buf, "PUT", 3)) {
        begin += 3;
    } else if (0 == strncasecmp(buf, "DELETE", 6)) {
        begin += 6;
    } else if (0 == strncasecmp(buf, "OPTIONS", 7)) {
        begin += 7;
    } else if (0 == strncasecmp(buf, "CONNECT", 7)) {
        begin += 7;
    } else if (0 == strncasecmp(buf, "HTTP/", 5)){
        isresponse = true;
        begin += 5;
    } else {
        return NULL;
    }
    if (!isresponse) {
        message->method = std::string(buf, begin - buf);
        begin += 1;
        end = strchr(begin, ' ');
        if (end == NULL || end + 9 >= buf + len) { // no http/1.*
            return NULL;
        }
        message->url = std::string(begin, end - begin);
        begin = end + 1 + 5; //skip space http/
    }

    if (0 == strncasecmp(begin, "1.1", 3)) {
        message->httpversion = ape::message::SHttpMessage::HTTP_1_1;
        message->keepalive = true;
    } else if (0 == strncasecmp(begin, "1.0", 3)) {
        message->httpversion = ape::message::SHttpMessage::HTTP_1_0;
        message->keepalive = false;
    } else {
        return NULL;
    }
    begin += 3; //skip 1.*
    if (isresponse) {
        if (*(begin++) != ' ') {
            return NULL;
        }
        message->SetReply(atoi(begin));
        //BS_XLOG(XLOG_TRACE,"CHttpParser::%s, code[%d], begin[%s]\n", __FUNCTION__, message->code, begin);
        begin = end + 1; //skip space

        begin = strstr(begin, "\r\n");
        if (begin == NULL || begin >= buf + len) {
            return NULL;
        }
    }
    begin += 2; //skip \r\n

    const char *headend = strstr(begin,"\r\n\r\n");
    if (headend == NULL || headend > buf + len) {
        return buf;  //incomplete packet
    }

    end = strstr(begin, "\r\n");
    while (end != NULL && (begin < headend) && end <= headend) {
        const char *colon = strchr(begin, ':');
        if (colon != NULL && colon < end) {
            std::string key = trim(begin, colon - begin, true);
            std::string value = trim(colon + 1, end - colon - 1);
            message->AddHeader(key, value);
            ParseHeader(key, value, message);
        }
        begin = end + 2;
        end = strstr(begin, "\r\n");
    }

    begin = headend + 4;
    if(ischunked_) {
        while(begin < buf + len) {
            int chunklen = 0;
            sscanf(begin, "%0X", &chunklen);
            if (1 != sscanf(begin, "%0X", &chunklen) || chunklen < 0) {
                return NULL;
            }
            begin = strstr(begin,"\r\n") + 2;
            if (chunklen == 0) {
                break;
            }
            if (begin + chunklen > buf + len) { //incomplete packet
                return buf;
            }
            message->body.append(begin, chunklen);
            begin += chunklen + 2;
        }
        return begin;
    }
    if (begin + contentlen_ > buf + len) { //incomplete packet
        return buf;
    }
    if (contentlen_ > 0) {
        //BS_XLOG(XLOG_TRACE,"CHttpParser::%s, contentlen_[%d], begin[%s]\n", __FUNCTION__, contentlen_,begin);
        message->body.assign(begin, contentlen_);
    }
    return begin + contentlen_;
}
int CHttpParser::Encode(const ape::message::SNetMessage *msg, ape::common::CBuffer *out) {
    return (msg->type == ape::message::SNetMessage::E_Request) ?
        EncodeRequest((ape::message::SHttpMessage *)msg, out) :
        EncodeResponse((ape::message::SHttpMessage *)msg, out);
}
void CHttpParser::ParseHeader(const std::string &key, const std::string &value, ape::message::SHttpMessage *message) {
    if (key.length() == 10 && 0 == strncasecmp(key.c_str(), "connection", 10)) {
        if (value.length() == 5 && 0 == strncasecmp(value.c_str(), "close", 5)) {
            message->keepalive = false;
        } else if(value.length() == 10 && 0 == strncasecmp(value.c_str(), "keep-alive", 10)) {
             message->keepalive = true;
        }
    } else if(key.length() == 15 && 0 == strncasecmp(key.c_str(), "x-forwarded-for", 15)) {
        std::string::size_type loc = value.find(',');
        message->xforwordip = loc == std::string::npos ? value : value.substr(0, loc);
    } else if(key.length() == 14 && 0 == strncasecmp(key.c_str(), "content-length", 14)) {
        contentlen_ = atoi(value.c_str());
    } else if(key.length() == 17 && 0 == strncasecmp(key.c_str(), "Transfer-Encoding", 17) &&
            value.length() == 7 && 0 == strncasecmp(value.c_str(), "chunked", 7)) {
        ischunked_ = true;
    } else if(key.length() == 10 && 0 == strncasecmp(key.c_str(), "Set-Cookie", 10)) {
        DecodeCookie(value.c_str(), value.length(), message);
    }
}
void CHttpParser::DecodeCookie(const char *buf, int len, ape::message::SHttpMessage *message) {
    const char *begin = buf;
    const char *end = buf + len;
    const char *split1 = begin, *split2 = begin;
    std::string strkey, strvalue, strpath, strdomain;
    time_t ex = 0;
    for (const char *p = begin; p < end && begin < end; ++p) {
        if (*p == '=') {
          split1 = p;
        } else if(*p == ';') {
          split2 = p;
        } else if(p == end - 1) {
          split2 = end;
        }

        if ( split1 > begin && split2 >= split1 + 1) {
            if (0 == strncasecmp(begin, "path", 4)) {
                strpath.append(split1 + 1, split2 - split1 -1);
            } else if (0 == strncasecmp(begin, "domain", 6)) {
                strdomain.append(split1 + 1, split2 - split1 -1);
            } else if (0 == strncasecmp(begin, "expires", 7)) {
                ex = httptime(std::string(split1 + 1, split2 - split1 -1).c_str());
            } else {
                strkey.append(begin, split1 - begin);
                strvalue.append(split1 + 1, split2 - split1 -1);
            }
            begin = split2 + 1;
            for (; begin < end && *begin == ' '; ++begin){}
        }
    }
    if (!strkey.empty()) {
        message->cookies[strkey] = ape::message::SHttpMessage::SCookie(strvalue, strpath, strdomain, ex);
    }
}
int CHttpParser::EncodeRequest(const ape::message::SHttpMessage *msg, ape::common::CBuffer *out) {
    default_header_list_.clear();
    std::map<std::string, std::string>::const_iterator itr = sm_default_request_header_.begin();
    for (; itr != sm_default_request_header_.end(); ++itr) {
        default_header_list_.push_back(itr->first);
    }

    out->append(msg->method.empty() ? "GET" : msg->method.c_str());
    out->append(" ");

    const char *begin = msg->url.c_str();
    const char *host = NULL;
    int hostlen = 0;
    if (0 == strncasecmp(begin, "http://", 7)) {
        host = begin + 7;
        begin = strchr(host, '/');
        if (begin == NULL) {
            hostlen = msg->url.c_str() + msg->url.length() - host;
        } else {
            hostlen = begin - host;
        }
    }
    out->append(begin == NULL || 0 == strlen(begin) ? "/" : begin);
    out->append(" ");
    out->append(msg->httpversion == ape::message::SHttpMessage::HTTP_1_1 ? "HTTP/1.1\r\n" : "HTTP/1.0\r\n");

    if (host != NULL && hostlen > 0) {
        out->append("Host: ");
        out->append(host, hostlen);
        out->append("\r\n");
    }
    boost::unordered_map<std::string, std::string>::const_iterator itrh = msg->headers.begin();
    for (; itrh != msg->headers.end(); ++itrh) {
        out->append(itrh->first.c_str());
        out->append(": ");
        out->append(itrh->second.c_str());
        out->append("\r\n");
        if (true == std::binary_search(default_header_list_.begin(), default_header_list_.end(), itrh->first)) {
            default_header_list_.remove(itrh->first);
        }
    }

    if (!msg->cookies.empty()) {
        out->append("Cookie: ");
    }
    boost::unordered_map<std::string, ape::message::SHttpMessage::SCookie>::const_iterator itrc = msg->cookies.begin();
    for (; itrc != msg->cookies.end(); ++itrc) {
        out->append(itrc->first.c_str());
        out->append("=");
        out->append(UrlEncoder::encode(itrc->second.value.c_str(), itrc->second.value.length()).c_str());
        out->append("; ");
    }
    if (!msg->cookies.empty()) {
        out->append("\r\n");
    }

    out->append("Connection: ");
    out->append(msg->keepalive ? "Keep-Alive" : "Close");
    out->append("\r\n");

    std::list<std::string>::iterator itrl = default_header_list_.begin();
    for (; itrl != default_header_list_.end(); ++itrl) {
        out->append(itrl->c_str());
        out->append(": ");
        out->append(sm_default_request_header_[*itrl].c_str());
        out->append("\r\n");
    }

    if (!msg->body.empty()) {
        char szlen[32] = {0};
        snprintf(szlen, 31, "%d", msg->body.length());
        out->append("Content-Length: ");
        out->append(szlen);
        out->append("\r\n\r\n");
        out->append(msg->body.c_str());
    } else {
        out->append("\r\n");
    }
    return 0;
}
int CHttpParser::EncodeResponse(const ape::message::SHttpMessage *msg, ape::common::CBuffer *out) {
    default_header_list_.clear();
    std::map<std::string, std::string>::const_iterator itr = sm_default_header_.begin();
    for (; itr != sm_default_header_.end(); ++itr) {
        default_header_list_.push_back(itr->first);
    }

    out->append(msg->httpversion == ape::message::SHttpMessage::HTTP_1_1 ? "HTTP/1.1 " : "HTTP/1.0 ");
    out->append(GetStatus(msg->code).c_str());
    out->append("\r\n");

    boost::unordered_map<std::string, std::string>::const_iterator itrh = msg->headers.begin();
    for (; itrh != msg->headers.end(); ++itrh) {
        out->append(itrh->first.c_str());
        out->append(": ");
        out->append(itrh->second.c_str());
        out->append("\r\n");
        if (true == std::binary_search(default_header_list_.begin(), default_header_list_.end(), itrh->first)) {
            default_header_list_.remove(itrh->first);
        }
    }

    boost::unordered_map<std::string, ape::message::SHttpMessage::SCookie>::const_iterator itrc = msg->cookies.begin();
    for (; itrc != msg->cookies.end(); ++itrc) {
        out->append("Set-Cookie: ");
        out->append(itrc->first.c_str());
        out->append("=");
        out->append(UrlEncoder::encode(itrc->second.value.c_str(), itrc->second.value.length()).c_str());
        out->append("; path=");
        out->append(itrc->second.path.c_str());
        if (!itrc->second.domain.empty()) {
            out->append("; domain=");
            out->append(itrc->second.domain.c_str());
        }
        if (itrc->second.expires > 0) {
            out->append("; expires=");
            out->append(httptime(itrc->second.expires).c_str());
        }
        out->append("\r\n");
    }

    std::list<std::string>::iterator itrl = default_header_list_.begin();
    for (; itrl != default_header_list_.end(); ++itrl) {
        out->append(itrl->c_str());
        out->append(": ");
        out->append(sm_default_header_[*itrl].c_str());
        out->append("\r\n");
    }

    out->append("Date: ");
    out->append(httptime(time(NULL)).c_str());
    out->append("\r\n");

    out->append("Connection: ");
    out->append(msg->keepalive ? "Keep-Alive" : "Close");
    out->append("\r\n");

    if (msg->httpversion == ape::message::SHttpMessage::HTTP_1_1) {
        out->append("Transfer-Encoding: chunked\r\n");
        out->append("\r\n");
        char szlen[32] = {0};
        snprintf(szlen, 31, "%0x", msg->body.length());
        out->append(szlen);
        out->append("\r\n");
        out->append(msg->body.c_str());
        out->append("\r\n0\r\n\r\n");
    } else {
        char szlen[32] = {0};
        snprintf(szlen, 31, "%d", msg->body.length());
        out->append("Content-Length: ");
        out->append(szlen);
        out->append("\r\n\r\n");
        out->append(msg->body.c_str());
    }
    return 0;
}
ape::message::SNetMessage *CHttpParser::CreateHeartBeatMessage(ape::message::SNetMessage::SMessageType type) {
    ape::message::SHttpMessage *msg = new ape::message::SHttpMessage;
    msg->keepalive = true;
    msg->httpversion = ape::message::SHttpMessage::HTTP_1_1;
    if (type == ape::message::SNetMessage::E_Request) {
        msg->method = "GET";
        msg->url = "/HeartBeat";
    } else {
        msg->SetReply(200);
    }
    return msg;
}


}
}

