#include "loghelper.h"
#include "controller.h"
#include "mongoclient.h"
#include "cjson.h"
#include <gtest/gtest.h>
#include <fstream>
#include <string>
#include <vector>
using std::string;
using std::vector;
using std::ifstream;

#ifdef Test_Yali_Mongo_From_File

using namespace ape::database;

void ParseJsons(const vector<string> &jsons, vector<bson_t*> *vec) {
    CJson2Bson convert;
    for (vector<string>::const_iterator itr = jsons.begin(); itr != jsons.end(); ++itr) {
        bson_t *bson = bson_new();
        if (0 != convert.Convert(*itr, bson)) {
            bson_destroy(bson);
            BS_XLOG(XLOG_WARNING, "illegal json: %s\n", itr->c_str());
            return;
        }
        vec->push_back(bson);
    }
}
void Execute(CMongoClient *client, const bson_t *bson) {
    const char *collection = "zampda_v2.material_new";
    std::vector<bson_t *> result;
    client->Query(&result, collection, bson, NULL, 0, 10);
    for (std::vector<bson_t *>::iterator itr = result.begin(); itr != result.end(); ++itr) {
        char *str = bson_as_json (*itr, NULL);
        BS_XLOG(XLOG_DEBUG, "[%s]\n", str);
        bson_free(str);
        bson_destroy(*itr);
    }
}
inline void FileGetContents(const string &strfile, vector<string> *vec) {
    ifstream fin;
    fin.open(strfile.c_str());
    if( !fin ) {
        BS_XLOG(XLOG_ERROR, "open file[%s] failed\n", strfile.c_str());
        return;
    }
    int MAX_LENGTH = 1024;
    char line[1024] = {0};
    int i = 0;
    while( fin.getline(line, MAX_LENGTH) && ++i <= 1000000) {
        unsigned int size = line[strlen(line) - 1] == '\r' ? strlen(line) - 1 : strlen(line);
        vec->push_back(string(line, size));
    }
}
TEST(CMongoClient, Yali) {
    CMongoClient client;
    client.Connect("mongodb://127.0.0.1/");


    vector<string> vec;
    FileGetContents("query.json", &vec);
    vector<bson_t *> bsons;
    ParseJsons(vec, &bsons);
    
    struct timeval begin, end;
    struct timezone tz;
    gettimeofday(&begin, &tz);
    int yalicount = bsons.size();
    BS_XLOG(XLOG_FATAL,"start, count[%d] time[%u,%u]\n",yalicount,begin.tv_sec,begin.tv_usec);
    
    int i = 1;
    struct timeval t1, t2;
    gettimeofday(&t1, &tz);
    int unit = 1;
    for (vector<bson_t *>::iterator itr = bsons.begin(); itr != bsons.end(); ++itr, ++i) {
        char *str = bson_as_json (*itr, NULL);
        BS_XLOG(XLOG_DEBUG, "[%s]\n", str);
        if (i % unit == 0) {
            gettimeofday(&t2, &tz);
            long inter = (t2.tv_sec - t1.tv_sec)*1000 + (long)(t2.tv_usec - t1.tv_usec)/1000;
            //BS_XLOG(XLOG_ERROR, "Execute Case[%d - %d] in [%ld]ms\n", i - unit, i, inter);
            t1.tv_sec = t2.tv_sec;
            t1.tv_usec = t2.tv_usec;
            if (inter > 50) { //when i == 1, test case which is not indexed in mongodb
                BS_XLOG(XLOG_ERROR, "Execute case in [%ld]ms [%s]\n", inter, str);
            }
        }
        Execute(&client, *itr);
        bson_free(str);
    }

    gettimeofday(&end, &tz);
    float interval = (end.tv_sec - begin.tv_sec) + (float)(end.tv_usec - begin.tv_usec)/1000000;
    BS_XLOG(XLOG_FATAL,"after, time[%u,%u], req/s[%.3f]\n\n",end.tv_sec,end.tv_usec,(float)yalicount/interval);
    
    for (vector<bson_t *>::iterator itr = bsons.begin(); itr != bsons.end(); ++itr) {
        bson_destroy(*itr);
    }
}


#endif
