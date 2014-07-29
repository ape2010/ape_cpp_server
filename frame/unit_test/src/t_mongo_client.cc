#include "loghelper.h"
#include "controller.h"
#include "mongoclient.h"
#include "cjson.h"
#include <gtest/gtest.h>

#ifdef Test_MongoClient

using namespace ape::database;

TEST(CMongoClient, Connect) {    
     
    CMongoClient client;
    client.Connect("mongodb://127.0.0.1/");
    const char *collection = "test.userinfo";

    bson_t keys;
    bson_init(&keys);
    BSON_APPEND_INT32(&keys, "name", 1);
    EXPECT_EQ(0, client.CreateIndex(collection, &keys));
    bson_destroy(&keys);
    
    EXPECT_EQ(0, client.DropIndex(collection, "name_1"));
}
TEST(CMongoClient, Insert) {
    CMongoClient client;
    client.Connect("mongodb://127.0.0.1/");
    const char *collection = "test2.userinfo2";

    bson_t doc;
    bson_init(&doc);
    BSON_APPEND_UTF8(&doc, "name", "gavin");
    BSON_APPEND_INT32(&doc, "age", 1);
    BSON_APPEND_UTF8(&doc, "job", "job");
    //EXPECT_EQ(0, client.Insert(collection, &doc));
    bson_destroy(&doc);
}
TEST(CMongoClient, Update) {
    CMongoClient client;
    client.Connect("mongodb://127.0.0.1/");
    const char *collection = "test.userinfo";

    bson_t selector, update, child;
    bson_init(&selector);
    bson_init(&update);
    BSON_APPEND_UTF8(&selector, "name", "gavin");
    BSON_APPEND_DOCUMENT_BEGIN(&update, "$inc", &child);
    BSON_APPEND_INT32(&child, "age", 1);
    bson_append_document_end(&update, &child);
    EXPECT_EQ(0, client.Update(collection, &selector, &update, true));
    bson_destroy(&selector);
    bson_destroy(&update);
}
TEST(CMongoClient, Query) {
      
    CMongoClient client;
    client.Connect("mongodb://127.0.0.1/");
    const char *collection = "test.userinfo";

    bson_t query;
    bson_init(&query);
    BSON_APPEND_UTF8(&query, "name", "gavin");
    std::vector<bson_t *> result;
    client.Query(&result, collection, &query);
    for (std::vector<bson_t *>::iterator itr = result.begin(); itr != result.end(); ++itr) {
        char *str = bson_as_json (*itr, NULL);
        BS_XLOG(XLOG_DEBUG, "%s\n", str);
        bson_free(str);
        bson_destroy(*itr);
    }
    bson_destroy(&query);
}
#endif
