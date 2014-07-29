#include "loghelper.h"
#include "controller.h"
#include "mongoclient.h"
#include "mongohelper.h"
#include "cjson.h"
#include <gtest/gtest.h>

#ifdef Test_MONGO_HELPER

using namespace ape::database;

TEST(CMongoClient, Connect) {
    uint32_t now = (uint32_t)time(NULL);
    bson_oid_t oid;
    bson_oid_init_from_time(&oid, now);
    char szid[25] = {0};
    bson_oid_to_string(&oid, szid);
    BS_XLOG(XLOG_DEBUG, "id[%s]\n\n", szid);
    
    bson_oid_t oid2;
    bson_oid_init_from_time(&oid2, now);
    char szid2[25] = {0};
    bson_oid_to_string(&oid2, szid2);
    BS_XLOG(XLOG_DEBUG, "id[%s]\n", szid2);
    //EXPECT_EQ(0, client.DropIndex(collection, "name_1"));
}
#endif
