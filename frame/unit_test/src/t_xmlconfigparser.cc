#include "gtest/gtest.h"
#include "xmlconfigparser.h"
#include "controller.h"
#include "loghelper.h"

#ifdef Test_XmlConfigParser
using namespace ape::common;
void ParseSosList(const std::string &str) {
    CXmlConfigParser parserdetail;
    parserdetail.ParseDetailBuffer(str.c_str());
    BS_XLOG(XLOG_DEBUG, "[%s]\n", parserdetail.GetParameter("ServiceId").c_str());
    
    std::vector<std::string> vec = parserdetail.GetParameters("ServerAddr");
    std::vector<std::string>::iterator itr = vec.begin();
    for (; itr != vec.end(); ++itr) {
        BS_XLOG(XLOG_DEBUG, "addr[%s]\n", itr->c_str());
    }
}
TEST(CXmlConfigParser, Dump) {
    CXmlConfigParser parser;
    EXPECT_EQ(0, parser.ParseFile("testfile/config.xml"));
    EXPECT_STREQ( "43210", parser.GetParameter("CohPort").c_str());
    EXPECT_EQ( 43210, parser.GetParameter("CohPort", 0));
    EXPECT_STREQ( "", parser.GetParameter("NULL").c_str());
    EXPECT_EQ( 10, parser.GetParameter("NULL", 10));
    EXPECT_STREQ("mothername", parser.GetParameter("family/mother/name", "name").c_str());
    EXPECT_EQ(50, parser.GetParameter("family/mother/age", 50));
    EXPECT_EQ(50, parser.GetParameter("/family/mother/age/", 50));
    EXPECT_STREQ("", parser.GetParameter("/family/mother/age/").c_str());
    
    std::vector<std::string> vec = parser.GetParameters("array/item");
    EXPECT_STREQ("aaa", vec[0].c_str());
    EXPECT_STREQ("bbb", vec[1].c_str());
    EXPECT_STREQ("ccc", vec[2].c_str());
    
    vec = parser.GetParameters("Sos/SosList");
    std::vector<std::string>::iterator itr = vec.begin();
    for (; itr != vec.end(); ++itr) {
        ParseSosList(*itr);
    }
    //BS_XLOG(XLOG_DEBUG, "[%s]\n", parser.GetParameter("Sos").c_str());
    //BS_XLOG(XLOG_DEBUG, "[%s]\n", parser.GetParameter("Sos/SosList").c_str());
    //BS_XLOG(XLOG_DEBUG, "[%s]\n", parser.GetString().c_str());
    
    CXmlConfigParser parser2;
    EXPECT_EQ(-1, parser2.ParseFile("testfile/badconfig.xml"));
    BS_XLOG(XLOG_DEBUG, "parse failed, [%s]\n", parser2.GetErrorMessage().c_str());
}
TEST(CXmlConfigParser, fail) {
    CXmlConfigParser parser2;
    EXPECT_EQ(-1, parser2.ParseFile("testfile/badconfig.xml"));
    BS_XLOG(XLOG_DEBUG, "parse failed, [%s]\n", parser2.GetErrorMessage().c_str());
    
    EXPECT_EQ(-1, parser2.ParseFile("testfile/nnn.xml"));
    BS_XLOG(XLOG_DEBUG, "parse failed, [%s]\n", parser2.GetErrorMessage().c_str());
}
#endif
