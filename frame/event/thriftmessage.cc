#include "thriftmessage.h"
#include "loghelper.h"

namespace ape {
namespace message {

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
std::string SThriftMessage::BriefInfo() {
    std::string info;
    info.append(method);
    char sz[64] = {0};
    snprintf(sz, 63, ", seq[%d], code[%d]", seqid, code);
    info.append(sz);
    return info;
}
void SThriftMessage::Dump() {
    BS_XLOG(XLOG_DEBUG,"SThriftMessage::%s\n%s\n",__FUNCTION__, NoticeInfo().c_str());
}


}
}