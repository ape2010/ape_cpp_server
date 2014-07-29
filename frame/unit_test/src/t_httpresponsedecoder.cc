#include "gtest/gtest.h"
#include "httpparser.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_HttpResponseDecoder
using namespace ape::common;

TEST(CHttpParser, ResponseDecode1) {
    const char *packet = "HTTP/1.1 200 ok\r\n\
 Date:Fri, 23 May 2014 06:36:18 GMT\r\n\
 Set-Cookie:BDSVRTM=38; path=/; expires=Fri, 23 May 2014 06:36:18 GMT\r\n\
 Set-Cookie:H_PS_PSSID=6225_6549_6249_1439_5225_6583_6505_6476_4760_6017_6427_6441_6529; path=/; domain=.baidu.com\r\n\
Connection:keep-alive\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_TRUE(message.keepalive);
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_EQ(200, message.code);
    EXPECT_STREQ(message.headers["date"].c_str(), "Fri, 23 May 2014 06:36:18 GMT");
    EXPECT_STREQ(message.headers["connection"].c_str(), "keep-alive");
    message.Dump();
}

//无HTTP版本号信息，error
TEST(CHttpParser, ResponseDecode2) {
    const char *packet = "HTTP/1.0 302 rssss\r\n\
 Date:Fri, 23 May 2014 06:36:18 GMT\r\n\
 Set-Cookie:BDSVRTM=38; path=/; expires=Fri, 23 May 2014 06:36:18 GMT\r\n\
 Set-Cookie:H_PS_PSSID=6225_6549_6249_1439_5225_6583_6505_6476_4760_6017_6427_6441_6529; path=/; domain=.baidu.com\r\n\
Connection:keep-alive\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_TRUE(message.keepalive);
    EXPECT_EQ(SHttpMessage::HTTP_1_0, message.httpversion);
    EXPECT_EQ(302, message.code);
    EXPECT_STREQ(message.headers["date"].c_str(), "Fri, 23 May 2014 06:36:18 GMT");
    EXPECT_STREQ(message.headers["connection"].c_str(), "keep-alive");
    message.Dump();
}
TEST(CHttpParser, ResponseDecode3) {
    const char *packet = "HTTP/1.1 302 rssss\r\n\
Content-Length:9\r\n\r\n\
post data";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_TRUE(message.keepalive);
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_EQ(302, message.code);
    EXPECT_STREQ(message.body.c_str(), "post data");
    message.Dump();
}
TEST(CHttpParser, ResponseDecode4) {
    const char *packet = "HTTP/1.1 302 rssss\r\n\
Transfer-Encoding:chunked\r\n\r\n\
9\r\n\
post data\r\n\
0\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_TRUE(message.keepalive);
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_EQ(302, message.code);
    EXPECT_STREQ(message.body.c_str(), "post data");
    message.Dump();
}
TEST(CHttpParser, ResponseDecode5) {
    const char *packet = "HTTP/1.1302rssss\r\n\
Transfer-Encoding:chunked\r\n\r\n\
9\r\n\
post data\r\n\
0\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(NULL, decoder.Decode(packet, len, &message));
}
TEST(CHttpParser, ResponseDecode6) {
    const char *packet = "HTTP/1.1 302rssss\r\n\
Transfer-Encoding:chunked\r\n\r\n\\r\n\
9\r\n\
post data\r\n\
0\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(NULL, decoder.Decode(packet, len, &message));
}
TEST(CHttpParser, ResponseDecode7) {
    const char *packet = "HTTP/1.1 302 rssss\r\n\
Transfer-Encoding:chunked\r\n\r\n\
20\r\n\
post data\r\n\
0\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet, decoder.Decode(packet, len, &message));
}

TEST(CHttpParser, ResponseDecode8) {
    const char *packet = "HTTP/1.1 302 rssss\r\n\
Content-Length:19\r\n\r\n\
post data";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet, decoder.Decode(packet, len, &message));
}

TEST(CHttpParser, ResponseDecode9) {
    const char *packet = "HTTP/1.1 302 rssss\r\n\
Content-Length:9\r\n\r\n\
post data";
    int len = strlen(packet);

    char sz[10240] = {0};
    memcpy(sz, packet, len);
    memcpy(sz + len, packet, len);
    memcpy(sz + 2 * len, packet, len / 2);
    CHttpParser decoder;
    SHttpMessage message;
    char *p = sz;
    for (int i = 0; i < 2; ++i) {
        char *last = p;
        p = (char *)decoder.Decode(p, strlen(p), &message);
        BS_XLOG(XLOG_DEBUG,"   ----------sz[%p], len[%d], last[%p], (last+len)[%p] p[%p], \n%s\n", 
            sz, len, last, last + len, p, p);
        EXPECT_EQ(last + len, p);
        EXPECT_TRUE(message.keepalive);
        EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
        EXPECT_EQ(302, message.code);
        EXPECT_STREQ(message.body.c_str(), "post data");
        message.Dump();
    }
    EXPECT_EQ(p, decoder.Decode(p, strlen(p), &message));
}
TEST(CHttpParser, ResponseDecode10) {
    const char *packet = "HTTP/1.1 302 rssss\r\n\
Transfer-Encoding:chunked\r\n\r\n\
9\r\n\
post data\r\n\
0\r\n";
    int len = strlen(packet);

    char sz[10240] = {0};
    memcpy(sz, packet, len);
    memcpy(sz + len, packet, len);
    memcpy(sz + 2 * len, packet, len / 2);
    CHttpParser decoder;
    char *p = sz;
    for (int i = 0; i < 2; ++i) {
        char *last = p;
        SHttpMessage message;
        p = (char *)decoder.Decode(p, strlen(p), &message);
        BS_XLOG(XLOG_DEBUG,"   -----%s-----sz[%p], len[%d], last[%p], (last+len)[%p] p[%p], \n%s\n", 
            __FUNCTION__, sz, len, last, last + len, p, p);
        EXPECT_EQ(last + len, p);
        EXPECT_TRUE(message.keepalive);
        EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
        EXPECT_EQ(302, message.code);
        EXPECT_STREQ(message.body.c_str(), "post data");
        message.Dump();
    }
    SHttpMessage message;
    EXPECT_EQ(p, decoder.Decode(p, strlen(p), &message));
}
#endif
