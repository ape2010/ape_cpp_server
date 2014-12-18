#include "ftdcoder.h"
#include "fieldbuffer.h"
#include <arpa/inet.h>

namespace protocol {

CFtdEncoder::CFtdEncoder() {
    memset(buf_.base(), 0, sizeof(SFtdHeader));
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->version = 1;
    header->chain = E_FTD_CHAIN_LAST;
    buf_.inc_loc(sizeof(SFtdHeader));
}
CFtdEncoder::CFtdEncoder(uint32_t tid, uint32_t requestid, uint32_t seqno, const void *body, int bodylen) {
    memset(buf_.base(), 0, sizeof(SFtdHeader));
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->version = 1;
    header->chain = E_FTD_CHAIN_LAST;
    header->transactionid = htonl(tid);
    header->requestid = htonl(requestid);
    header->seqno = htonl(seqno);
    buf_.inc_loc(sizeof(SFtdHeader));
    SetBody(body, bodylen);
}
CFtdEncoder::CFtdEncoder(uint32_t tid, uint32_t requestid, uint32_t seqno, EFtdStreamType streamtype, EFtdChainType chain, uint8_t version, const void *body, int bodylen) {
    memset(buf_.base(), 0, sizeof(SFtdHeader));
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->version = 1;
    header->chain = E_FTD_CHAIN_LAST;
    header->transactionid = htonl(tid);
    header->requestid = htonl(requestid);
    header->seqno = htonl(seqno);
    header->streamtype = htons(streamtype);
    header->version = version;
    header->chain = chain;
    buf_.inc_loc(sizeof(SFtdHeader));
    SetBody(body, bodylen);
}
void CFtdEncoder::SetVersion(uint8_t version) {
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->version = 1;
}
void CFtdEncoder::SetChain(EFtdChainType chain) { 
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->chain = chain;
}
void CFtdEncoder::SetStreamType(EFtdStreamType type) { 
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->streamtype = htons(type);
}
void CFtdEncoder::SetTid(uint32_t tid) { 
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->transactionid = htonl(tid);
}
void CFtdEncoder::SetSeqNo(uint32_t seqno) { 
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->seqno = htonl(seqno);
}
void CFtdEncoder::SetSequestId(uint32_t requestid) { 
    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->requestid = htonl(requestid);
}

void CFtdEncoder::AddField(const void *data, uint16_t len) { 
    if (len <= 0 || data == NULL) {
        return;
    }
    if (buf_.capacity() < len + sizeof(SFtdHeader)) {
        buf_.add_capacity(len + sizeof(SFtdHeader) + buf_.capacity());
    }

    SFtdHeader *header = (SFtdHeader *)buf_.base();
    uint16_t bodylen = ntohs(header->bodylen);
    char *loc = buf_.base() + sizeof(SFtdHeader) + bodylen;
    memcpy(loc, data, len);

    uint16_t fieldcount = ntohs(header->fieldcount) + 1;
    header->fieldcount = htons(fieldcount);

    header->bodylen = htons(len + bodylen);
    buf_.inc_loc(len);
}
void CFtdEncoder::SetBody(const void *data, int len) {
    if (len <= 0 || data == NULL) {
        return;
    }
    if (buf_.capacity() < len) {
        buf_.add_capacity(len + buf_.capacity());
    }
    
    uint16_t fieldcount = 0;
    const char *p = (const char *)data;
    const char *end = (const char *)data + len;
    while(p < end) { 
        SFieldHeader *fieldheader = (SFieldHeader *)p;
        p += ntohs(fieldheader->len) + sizeof(SFieldHeader);
        ++fieldcount;
    }

    SFtdHeader *header = (SFtdHeader *)buf_.base();
    header->fieldcount = htons(fieldcount);
    header->bodylen = htons(len);

    char *headend = buf_.base() + sizeof(SFtdHeader);
    memcpy(headend, data, len);
    buf_.inc_loc(len);
}
}
