#ifndef _TOOL_URL_CODE_H_
#define _TOOL_URL_CODE_H_
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

class UrlEncoder {
 public:
  static std::string encode(const char *pValue, size_t len){
    std::string ret = "";
    size_t i=0;
    for (i=0;i<len;i++) {
      if (isOrdinaryChar(pValue[i])) {
        ret += pValue[i];
      } else if (pValue[i] == ' ') {
        ret += "+";
      } else {
        char tmp[6]={0};
        sprintf(tmp,"%%%x",(unsigned char)pValue[i]);
        ret+= tmp;
      }
    }
    return ret;
  }
 private:
  static bool isOrdinaryChar(char c) {
    char ch = tolower(c);
    if ((ch>='a'&&ch<='z') || (ch>='0'&&ch<='9')) {
      return true;
    }
    return false;
  }
};

class URLDecoder {
public:
  static std::string decode(const char *pValue, size_t len) {
    std::string ret = "";
    URLDecoder::decode(pValue, len, &ret);
    return ret;
  }
  static void decode(const char *pValue, size_t len, std::string *out) {
    size_t i=0;
    for (i=0;i<len;i++) {
      if (pValue[i] == '+') {
        out->append(" ");
      } else if(pValue[i] == '%') {
        char tmp[4];
        char hex[4];      
        hex[0] = pValue[++i];
        hex[1] = pValue[++i];
        hex[2] = '\0';    
        sprintf(tmp,"%c",convertToDec(hex));
        out->append(tmp);
      } else {
        out->append(1, pValue[i]);
      }
    }
  }
  
  static int convertToDec(const char* hex) {
    char buff[12];
    sprintf(buff,"%s",hex);
    int ret = 0;
    size_t len = strlen(buff);
    size_t i=0;
    size_t j=0;
    for(i=0;i<len;i++) {
      char tmp[4];
      tmp[0] = buff[i];
      tmp[1] = '\0';
      getAsDec(tmp);
      int tmp_i = atoi(tmp);
      int rs = 1;
      for(j=i;j<(len-1);j++) {
        rs *= 16;
      }
      ret += (rs * tmp_i);
    }
    return ret;
  }

  static void getAsDec(char* hex) {
    char tmp = tolower(hex[0]);
    if (tmp == 'a') {
      strcpy(hex,"10");
    } else if(tmp == 'b') {
      strcpy(hex,"11");
    } else if(tmp == 'c') {
      strcpy(hex,"12");
    } else if(tmp == 'd') {
      strcpy(hex,"13");
    } else if(tmp == 'e') {
      strcpy(hex,"14");
    } else if(tmp == 'f') {
      strcpy(hex,"15");
    } else if(tmp == 'g') {
      strcpy(hex,"16");
    }
  }

};
#endif

