#include "fieldbuffer.h"
#include <arpa/inet.h>

namespace protocol {

CField::CField() { 
    memset(buf_.base(), 0, sizeof(SFieldHeader));
    buf_.inc_loc(sizeof(SFieldHeader));
}
CField::CField(uint16_t fieldid) {
    memset(buf_.base(), 0, sizeof(SFieldHeader));
    buf_.inc_loc(sizeof(SFieldHeader));
    SetFieldId(fieldid);
}
void CField::SetFieldId(uint16_t fieldid) { 
    SFieldHeader *header = (SFieldHeader *)buf_.base();
    header->fieldid = htons(fieldid);
}
void CField::AddValue(int32_t value) {
    int32_t netvalue = htonl(value);
    AddValue((const char *)&netvalue, 4, 4);
}
void CField::AddUint16(uint16_t value) {
    int32_t netvalue = htonl(value);
    AddValue((const char *)&netvalue, 2, 2);
}
void CField::AddUint8(uint8_t value) {
    int32_t netvalue = htonl(value);
    AddValue((const char *)&netvalue, 1, 1);
}
void CField::AddValue(const std::string &value, int32_t maxlen) { 
    AddValue(value.c_str(), value.length(), maxlen + 1);
}
void CField::AddValue(const char *value, int32_t maxlen) {
    AddValue(value, strlen(value), maxlen + 1);
}
void CField::AddValue(const void *value, int32_t len, int32_t maxlen) {
    if (buf_.capacity() < maxlen + sizeof(SFieldHeader)) {
        buf_.add_capacity(maxlen + sizeof(SFieldHeader) + buf_.capacity());
    }
    SFieldHeader *header = (SFieldHeader *)buf_.base();
    uint16_t fieldlen = ntohs(header->len);
    char *loc = buf_.base() + sizeof(SFieldHeader) + fieldlen;
    if (len < maxlen) { 
        memcpy(loc, value, len);
        memset(loc + len, 0, maxlen - len);
    } else { 
        memcpy(loc, value, maxlen);
    }
    header->len = htons(fieldlen + maxlen);
    buf_.inc_loc(maxlen);
}

}
