#ifndef _APELUS_MONITOR_LOG_HELPER_H_
#define _APELUS_MONITOR_LOG_HELPER_H_

#include "LogManager.h"
#include <cstring>
#include <stdio.h>

const int BUSINESS_MODULE = 102;
DEFINE_MODULE_XLOG(BUSINESS_MODULE, BS_XLOG)
#define BS_SLOG(Level,Event) SLOG(BUSINESS_MODULE,Level,Event)

const int SELF_CHECK_MODULE = 91;
DEFINE_MODULE_XLOG(SELF_CHECK_MODULE,SELF_CHECK_XLOG)


static inline char GetVisibleAscii(char c) { return (c >= 33 && c <= 126) ? c : '.'; }
static std::string GetBinaryDumpInfo(const char *buf, int len, int indent = 4) {
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

#endif


