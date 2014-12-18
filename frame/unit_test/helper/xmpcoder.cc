#include "xmpcoder.h"
#include <arpa/inet.h>

namespace protocol {
CXmpEncoder::CXmpEncoder() {
    memset(buf_.base(), 0, sizeof(SXmpHeader));
    buf_.reset_loc(sizeof(SXmpHeader));
}
CXmpEncoder::CXmpEncoder(EXmpType type) {
    memset(buf_.base(), 0, sizeof(SXmpHeader));
    SXmpHeader *header = (SXmpHeader *)buf_.base();
    header->type = type;
    buf_.reset_loc(sizeof(SXmpHeader));
}
void CXmpEncoder::SetXmpType(EXmpType type) {
    SXmpHeader *header = (SXmpHeader *)buf_.base();
    header->type = type;
}
void CXmpEncoder::SetExHeader(EXmpTagType tag, const void *data, int datalen) {
    SXmpHeader *header = (SXmpHeader *)buf_.base();
    uint16_t bodylen = ntohs(header->bodylen);
    int len = datalen + 2;
    if (buf_.capacity() < len) {
        buf_.add_capacity(len + buf_.capacity());
    }

    char *headend = buf_.base() + sizeof(SXmpHeader) + header->extheaderlen;
    if (bodylen > 0) {
        memmove(headend + len, headend, bodylen);
    }

    uint8_t tmplen = (uint8_t)datalen;
    memcpy(headend, &tag, 1);
    memcpy(++headend, (char *)(&tmplen), 1);
    memcpy(++headend, data, datalen);
    header->extheaderlen += (uint8_t)len;
    buf_.inc_loc(len);
}

void CXmpEncoder::SetBody(const void *data, int len) {
    SXmpHeader *header = (SXmpHeader *)buf_.base();
    if (buf_.capacity() < len) {
        buf_.add_capacity(len + buf_.capacity());
    }

    char *headend = buf_.base() + sizeof(SXmpHeader) + header->extheaderlen;
    memcpy(headend, data, len);
    header->bodylen = htons(len);
    buf_.inc_loc(len);
}



CXmpDecoder::CXmpDecoder(const void *data, int len) : body_(NULL) {
    memset(&header_, 0, sizeof(SXmpHeader));
    memset(&ext_header_, 0, sizeof(SXmpExtHeader));
    Decode(data, len);
}
int CXmpDecoder::Decode(const void *data, int len) {
    if (data == NULL || len == 0) {
        return -100;
    }
    if (len < sizeof(SXmpHeader)) {
        return -1;
    }
    memcpy(&header_, data, sizeof(SXmpHeader));
    header_.bodylen = ntohs(header_.bodylen);

    if (len < sizeof(SXmpHeader) + header_.extheaderlen + header_.bodylen) {
        return -1;
    }
    const char *headend = (const char *)data + sizeof(SXmpHeader);

    if (header_.extheaderlen > 0) {
        memcpy(&ext_header_, headend, 2);
        if (ext_header_.taglen + 2 == header_.extheaderlen) {
            return -100;
        }
        memcpy(ext_header_.data, headend + 2, ext_header_.taglen);
    }

    body_ = headend + header_.extheaderlen;
    return header_.bodylen;
}

}