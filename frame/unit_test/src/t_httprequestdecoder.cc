#include "gtest/gtest.h"
#include "httpparser.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_HttpRequestDecoder
using namespace ape::protocol;
using namespace ape::message;
using namespace ape::common;

TEST(CHttpParser, Decode1) {
    const char *packet = "GET /ECard/QueryInfo.fcgi?a=1&b=2&c=3 HTTP/1.1\r\n\
 Host : 192.168.90.22:10091  \r\n\
User-Agent:Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.15) Gecko/20110303 Firefox/3.6.15 (.NET CLR 3.5.30729)\r\n\
Accept:text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8\r\n\
Accept-Language:zh-cn,zh;q=0.5\r\n\
Accept-Encoding:gzip,deflate\r\n\
Accept-Charset:GB2312,utf-8;q=0.7,*;q=0.7\r\n\
Keep-Alive:115\r\n\
Connection:keep-alive\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_TRUE(message.keepalive);
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_STREQ(message.method.c_str(), "GET");
    EXPECT_STREQ(message.url.c_str(), "/ECard/QueryInfo.fcgi?a=1&b=2&c=3");
    EXPECT_STREQ(message.headers["host"].c_str(), "192.168.90.22:10091");
    EXPECT_STREQ(message.headers["user-agent"].c_str(), "Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.15) Gecko/20110303 Firefox/3.6.15 (.NET CLR 3.5.30729)");
    EXPECT_STREQ(message.headers["accept"].c_str(), "text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8");
    EXPECT_STREQ(message.headers["accept-language"].c_str(), "zh-cn,zh;q=0.5");
    EXPECT_STREQ(message.headers["accept-encoding"].c_str(), "gzip,deflate");
    EXPECT_STREQ(message.headers["accept-charset"].c_str(), "GB2312,utf-8;q=0.7,*;q=0.7");
    EXPECT_STREQ(message.headers["keep-alive"].c_str(), "115");
    EXPECT_STREQ(message.headers["connection"].c_str(), "keep-alive");
    message.Dump();
}
//无HTTP版本号信息，error
TEST(CHttpParser, Decode2) {
    const char *packet = "GET /ECard/QueryInfo.fcgi?a=1&b=2&c=3HTTP/1.0\r\n\
Host:192.168.90.22:10091\r\n\
User-Agent:Mozilla/5.0 (Windows; U; Windows NT 5.1; zh-CN; rv:1.9.2.15) Gecko/20110303 Firefox/3.6.15 (.NET CLR 3.5.30729)\r\n\
Accept:text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8\r\n\
Accept-Language:zh-cn,zh;q=0.5\r\n\
Accept-Encoding:gzip,deflate\r\n\
Accept-Char\r\n\
Keep-Alive:115\r\n\
Connection:Closen\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(NULL, decoder.Decode(packet, len, &message));
    //message.Dump();
}
//bad http head, default keepalive is true;
TEST(CHttpParser, Decode3) {
    const char *packet = "GET /ECard/QueryInfo.fcgi HTTP/1.1\r\n\
Host:192.168.90.22:10091\r\n\
Accept:text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8\r\n\
Accept-Language:zh-cn,zh;q=0.5\rn\
Accept-Encoding:gzip,deflate\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_TRUE(message.keepalive);
    EXPECT_STREQ(message.headers["accept-language"].c_str(), "zh-cn,zh;q=0.5\rnAccept-Encoding:gzip,deflate");
    message.Dump();
}
//default keepalive is false;
TEST(CHttpParser, Decode4) {
    const char *packet = "GET /ECard/QueryInfo.fcgi HTTP/1.0\r\n\
Host:192.168.90.22:10091\r\n\
Accept:text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8\r\n\
Accept-Language:zh-cn,zh;q=0.5\rn\
Accept-Encoding:gzip,deflate\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_FALSE(message.keepalive);
    EXPECT_STREQ(message.url.c_str(), "/ECard/QueryInfo.fcgi");
    message.Dump();
}
//keepalive is true;
TEST(CHttpParser, Decode5) {
    const char *packet = "GET /ECard/QueryInfo.fcgi HTTP/1.0\r\n\
Host:192.168.90.22:10091\r\n\
Accept:text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8\r\n\
Accept-Language:zh-cn,zh;q=0.5\r\n\
Accept-Encoding:gzip,deflate\r\n\
X-Forwarded-For:192.168.1.1,1.1.1.1\r\n\
Connection:keep-alive\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_EQ(SHttpMessage::HTTP_1_0, message.httpversion);
    EXPECT_TRUE(message.keepalive);
    EXPECT_STREQ(message.url.c_str(), "/ECard/QueryInfo.fcgi");
    EXPECT_STREQ(message.xforwordip.c_str(), "192.168.1.1");
    message.Dump();
}
//keepalive is false;
TEST(CHttpParser, Decode6) {
    const char *packet = "GET /ECard/QueryInfo.fcgi HTTP/1.1\r\n\
Host:192.168.90.22:10091\r\n\
Accept:text/html,application/xhtml+xml,application/xml;q=0.9,;q=0.8\r\n\
Accept-Language:zh-cn,zh;q=0.5\r\n\
Accept-Encoding:gzip,deflate\r\n\
X-Forwarded-For:192.168.1.1\r\n\
Connection:Close\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_FALSE(message.keepalive);
    EXPECT_STREQ(message.url.c_str(), "/ECard/QueryInfo.fcgi");
    EXPECT_STREQ(message.xforwordip.c_str(), "192.168.1.1");
    message.Dump();
}
TEST(CHttpParser, Decode7) {
    const char *packet = "POST /command HTTP/1.1\r\n\
Content-Length:9\r\n\r\n\
post data";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet + len, decoder.Decode(packet, len, &message));
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_TRUE(message.keepalive);
    EXPECT_STREQ(message.method.c_str(), "POST");
    EXPECT_STREQ(message.url.c_str(), "/command");
    EXPECT_STREQ(message.body.c_str(), "post data");
    message.Dump();
}
TEST(CHttpParser, Decode8) { //content-length is bigger incomplete packet
    const char *packet = "POST /command HTTP/1.1\r\n\
Content-Length:16\r\n\r\n\
post data";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet, decoder.Decode(packet, len, &message));
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_TRUE(message.keepalive);
    EXPECT_STREQ(message.method.c_str(), "POST");
    EXPECT_STREQ(message.url.c_str(), "/command");
    //EXPECT_STREQ(message.body.c_str(), "post data");
    message.Dump();
}
TEST(CHttpParser, Decode9) { //content-length is bigger
    const char *packet = "POST /command HTTP/1.1\r\n\
Content-Length:16\r\n\r\n";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(packet, decoder.Decode(packet, len, &message));
    EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
    EXPECT_TRUE(message.keepalive);
    EXPECT_STREQ(message.method.c_str(), "POST");
    EXPECT_STREQ(message.url.c_str(), "/command");
    EXPECT_STREQ(message.body.c_str(), "");
    message.Dump();
}
TEST(CHttpParser, Decode10) {
    const char *packet = "alsdkjfiesalkfsadfafadsf";

    int len = strlen(packet);
    CHttpParser decoder;
    SHttpMessage message;
    EXPECT_EQ(NULL, decoder.Decode(packet, len, &message));
}

TEST(CHttpParser, Decode11) {
    const char *packet = "POST /command HTTP/1.1\r\n\
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
        EXPECT_EQ(SHttpMessage::HTTP_1_1, message.httpversion);
        EXPECT_TRUE(message.keepalive);
        EXPECT_STREQ(message.method.c_str(), "POST");
        EXPECT_STREQ(message.url.c_str(), "/command");
        EXPECT_STREQ(message.body.c_str(), "post data");
        message.Dump();
    }
    EXPECT_EQ(p, decoder.Decode(p, strlen(p), &message));
}
#endif
