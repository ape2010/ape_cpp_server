#include "gtest/gtest.h"
#include "httpparser.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_HttpRequestEncoder
using namespace ape::common;

TEST(CHttpParser, EncodeRequest1) {
    CHttpParser encoder;
    SHttpMessage message;
    message.code = 0;
    message.keepalive = true;
    message.method = "POST";
    message.httpversion = SHttpMessage::HTTP_1_1;
    message.url = "http://11.1.1.1:9000/hello";
    message.body = "testbody";
    
    CBuffer buf;
    EXPECT_EQ(0, encoder.Encode(&message, &buf));
    message.Dump();
    BS_XLOG(XLOG_DEBUG,"%s request:\n%s\n",__FUNCTION__, std::string(buf.base(), buf.len()).c_str());
}
TEST(CHttpParser, EncodeRequest2) {
    CHttpParser encoder;
    SHttpMessage message;
    message.code = 0;
    message.keepalive = true;
    message.httpversion = SHttpMessage::HTTP_1_1;
    message.url = "http://11.1.1.1:9000";
    message.body = "testbody";
    
    CBuffer buf;
    EXPECT_EQ(0, encoder.Encode(&message, &buf));
    message.Dump();
    BS_XLOG(XLOG_DEBUG,"%s request:\n%s\n",__FUNCTION__, std::string(buf.base(), buf.len()).c_str());
}
TEST(CHttpParser, EncodeRequest3) {
    CHttpParser encoder;
    SHttpMessage message;
    message.code = 0;
    message.keepalive = false;
    message.httpversion = SHttpMessage::HTTP_1_0;
    message.url = "http://";
    
    CBuffer buf;
    EXPECT_EQ(0, encoder.Encode(&message, &buf));
    message.Dump();
    BS_XLOG(XLOG_DEBUG,"%s request:\n%s\n",__FUNCTION__, std::string(buf.base(), buf.len()).c_str());
}
TEST(CHttpParser, EncodeRequest4) {
    CHttpParser encoder;
    SHttpMessage message;
    message.code = 0;
    message.keepalive = false;
    message.httpversion = SHttpMessage::HTTP_1_0;
    message.body = "testbody";
    
    message.AddHeader("Content-Encoding", "gzip");
    message.AddHeader("Accept-Language", "Accept-LanguageAccept-Language");
    
    message.SetCookie("BDSVRTM", "aaaaaaa", "/", "", time(NULL) + 3600);
    message.SetCookie("H_PS_PSSID", "6225_6549_6249_1439_5225", "/", ".baidu.com");
    
    CBuffer response;
    EXPECT_EQ(0, encoder.Encode(&message, &response));
    message.Dump();
    BS_XLOG(XLOG_DEBUG,"%s response:\n%s\n",__FUNCTION__, std::string(response.base(), response.len()).c_str());
    
}

#endif
