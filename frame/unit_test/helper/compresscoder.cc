#include "compresscoder.h"
#include "CompressUtil.h"
#include <arpa/inet.h>

namespace protocol {
CCompressEncoder::CCompressEncoder(ECompressMethod method, EProtocolType type, const void *body, int bodylen) {
    SCompressHeader *header = (SCompressHeader *)buf_.base();
    header->type = (uint8_t)type;
    header->method = (uint8_t)method;

    buf_.reset_loc(sizeof(SCompressHeader));
    SetBody(body, bodylen);
}

void CCompressEncoder::SetBody(const void *data, int len) {
    if (len <= 0 || data == NULL) {
        return;
    }

    if (buf_.capacity() < len) {
        buf_.add_capacity(len + buf_.capacity());
    }

    SCompressHeader *header = (SCompressHeader *)buf_.base();
    unsigned char *headend = (unsigned char *)(buf_.base() + sizeof(SCompressHeader));
    if (header->method == E_COMPRESS_ZERO) {
        unsigned long outlen = 0;
        CompressUtil::Zerocompress((const unsigned char*)data, (long unsigned int)len, headend, outlen);
        buf_.inc_loc(outlen);
    } else {
        memcpy(headend, data, len);
        buf_.inc_loc(len);
    }
}


CCompressDecoder::CCompressDecoder(const void *data, int len) : body_(NULL) {
    memset(&header_, 0, sizeof(SCompressHeader));
    Decode(data, len);
}
int CCompressDecoder::Decode(const void *data, int len) {
    if (data == NULL || len == 0) {
        return -100;
    }
    if (len < sizeof(SCompressHeader)) {
        return -1;
    }
    memcpy(&header_, data, sizeof(SCompressHeader));
    body_ = (const char *)data + sizeof(SCompressHeader);
    int bodylen = len - sizeof(SCompressHeader);

    if (header_.method == E_COMPRESS_ZERO) {
        unsigned long outlen = 0;
        CompressUtil::Zerodecompress((const unsigned char*)body_, (long unsigned int)bodylen, (unsigned char*)(buf_.base()), outlen);
        buf_.reset_loc(outlen);
        body_ = buf_.base();
        return outlen;
    }

    return bodylen;
}

}