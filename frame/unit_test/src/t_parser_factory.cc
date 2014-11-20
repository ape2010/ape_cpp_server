#include <gtest/gtest.h>
#include "controller.h"

#ifdef Test_ParserFactory

#include "loghelper.h"
#include "baseparser.h"

TEST(ParserFactory, CreateParser) {
    ape::protocol::CBaseParser *parser = ape::protocol::ParserFactory::CreateParser(ape::protocol::E_PROTOCOL_HTTP);
    BS_XLOG(XLOG_DEBUG, "%s, parser[0X%0X]\n", typeid(this).name(), parser);
    EXPECT_NE(NULL, (long)parser);
}
#endif
