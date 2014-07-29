#ifndef _APE_COMMON_XML_CONFIG_PARSER_H_
#define _APE_COMMON_XML_CONFIG_PARSER_H_
#include <string>
#include <vector>
#include "tinyxml2.h"
namespace ape{
namespace common{
class CXmlConfigParser {
  public:
    CXmlConfigParser();
    int ParseFile(const std::string &file);
    int ParseBuffer(const char *buffer);
    int ParseDetailBuffer(const char *buffer);
    std::string GetParameter(const std::string &path);
    int GetParameter(const std::string &path, int defaultvalue);
    std::string GetParameter(const std::string &path, const std::string& defaultvalue);
    std::vector<std::string> GetParameters(const std::string & path);
    const std::string& GetErrorMessage()const{return error_;}
    std::string GetString();
  private:
    void SetXmlError();
  private:
    tinyxml2::XMLDocument xmldoc_;
    tinyxml2::XMLElement *root_;
    std::string error_;
};
}
}
#endif