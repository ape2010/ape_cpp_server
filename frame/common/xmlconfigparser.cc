#include "xmlconfigparser.h"
#include <algorithm>
namespace ape{
namespace common{
CXmlConfigParser::CXmlConfigParser() : root_(NULL) {
}
void CXmlConfigParser::SetXmlError() {
    if (xmldoc_.Error()) {
        char szcode[16] = {0};
        snprintf(szcode, 15, "id=%d ", xmldoc_.ErrorID());
        error_ = "XMLDocument error ";
        error_.append(szcode);
        error_.append(tinyxml2::GetError(xmldoc_.ErrorID()));
        error_.append(" str1=");
        if (xmldoc_.GetErrorStr1()) {
            error_.append(xmldoc_.GetErrorStr1());
        }
        error_.append(" str2=");
        if (xmldoc_.GetErrorStr2()) {
            error_.append(xmldoc_.GetErrorStr2());
        }
    }
}
int CXmlConfigParser::ParseFile(const std::string &file) {
    if (tinyxml2::XML_SUCCESS != xmldoc_.LoadFile(file.c_str())) {
        SetXmlError();
        return -1;
    }
    if( NULL == (root_ = xmldoc_.RootElement())) {
        error_ = "no root node!";
        return -1;
    }
    return 0;
}
int CXmlConfigParser::ParseBuffer(const char *buffer) {
    xmldoc_.Parse(buffer);
    if (xmldoc_.Error()) {
        SetXmlError();
        return -1;
    }
    if( NULL == (root_ = xmldoc_.RootElement())) {
        error_ = "no root node!";
        return -1;
    }
    return 0;
}
int CXmlConfigParser::ParseDetailBuffer(const char *buffer) {
    std::string str = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?><parameters>";
    str.append(buffer);
    str.append("</parameters>");
    return ParseBuffer(str.c_str());
}
std::string CXmlConfigParser::GetString() {
    tinyxml2::XMLPrinter printer;
    xmldoc_.Accept(&printer);
    return printer.CStr();
}
int CXmlConfigParser::GetParameter(const std::string &path, int defaultvalue) {
    std::string result = GetParameter(path);
    return result.empty() ? defaultvalue : atoi(result.c_str());
}
std::string CXmlConfigParser::GetParameter(const std::string &path, const std::string& defaultvalue) {
    std::string result = GetParameter(path);
    return result.empty() ? defaultvalue : result;
}
std::string CXmlConfigParser::GetParameter(const std::string &path) {
    tinyxml2::XMLElement *ele = root_;
    if (ele == NULL){
        return "";
    }
    std::string key;
    std::size_t begin = 0, end = 0;
    while (std::string::npos != (end = path.find("/", begin))) {
        key = path.substr(begin, end - begin);
        begin = end + 1;
        if (key.empty()) {
            return "";
        }
        if (NULL == (ele = ele->FirstChildElement(key.c_str()))) {
            return ""; 
        }
    }
    key = path.substr(begin);
    if(key.empty() || NULL == (ele = ele->FirstChildElement(key.c_str()))) {
        return "";
    }
    tinyxml2::XMLPrinter printer;
    ele->InnerAccept(&printer);
    return printer.CStr();
}
std::vector<std::string> CXmlConfigParser::GetParameters(const std::string &path) {
    std::vector<std::string> result;
    tinyxml2::XMLElement *ele = root_;
    if (ele == NULL){
        return result;
    }
    std::string key;
    size_t begin = 0, end = 0;
    while (std::string::npos != (end = path.find("/", begin))) {
        key = path.substr(begin, end - begin);
        begin = end + 1;
        if (key.empty()) {
            return result;
        }
        if (NULL == (ele = ele->FirstChildElement(key.c_str()))) {
            return result; 
        }
    }
    key = path.substr(begin);
    if(key.empty() || NULL == (ele = ele->FirstChildElement(key.c_str()))) {
        return result;
    }
    do {
        tinyxml2::XMLPrinter printer;
        ele->InnerAccept(&printer);
        result.push_back(printer.CStr());
    } while(NULL != (ele = ele->NextSiblingElement(key.c_str())));
    
    return result;
}

}
}
