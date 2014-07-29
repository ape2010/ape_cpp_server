#include "loghelper.h"
#include "controller.h"
#include "mongoclient.h"
#include "cjson.h"
#include <gtest/gtest.h>

#ifdef Test_Yali_Mongo

using namespace ape::database;

void YaliMongo(CMongoClient *client, const char *collection, const char *json) {
    CJson2Bson convert;
    bson_t *bson = bson_new();
    if (0 != convert.Convert(json, bson)) {
        bson_destroy(bson);
        BS_XLOG(XLOG_WARNING, "illegal json: %s\n", json);
        return;
    }
    
    struct timeval before, end;
    struct timezone tz;
    gettimeofday(&before, &tz);
    int yalicount = 100000;
    BS_XLOG(XLOG_FATAL,"test[%s]\n",json);
    BS_XLOG(XLOG_FATAL,"start, count[%d] time[%u,%u]\n",yalicount,before.tv_sec,before.tv_usec);
    for(int i = 0; i < yalicount; ++i) {
        std::vector<bson_t *> result;
        client->Query(&result, collection, bson, NULL, 0, 10);
        for (std::vector<bson_t *>::iterator itr = result.begin(); itr != result.end(); ++itr) {
            char *str = bson_as_json (*itr, NULL);
            //BS_XLOG(XLOG_DEBUG, "%s\n", str);
            bson_free(str);
            bson_destroy(*itr);
        }
    }
    gettimeofday(&end, &tz);
    float interval = (end.tv_sec - before.tv_sec) + (float)(end.tv_usec - before.tv_usec)/1000000;
    BS_XLOG(XLOG_FATAL,"after, time[%u,%u], req/s[%.3f]\n\n",end.tv_sec,end.tv_usec,(float)yalicount/interval);
    
    bson_destroy(bson);
}
TEST(CMongoClient, Yali) {
    CMongoClient client;
    client.Connect("mongodb://127.0.0.1/");
    const char *collection = "zampda_v2.material_new";

    YaliMongo(&client, collection, "{\"company_id\":8,\"product_id\":\"217381478\"}");
    YaliMongo(&client, collection, "{ \"company_id\" : 75 , \"product_id\" : { \"$in\" : [ \"100-506651\" , \"100-205209\" , \"100-209670\" , \"100-550507\" , \"100-506652\"]}}");
}
 
   

#endif
