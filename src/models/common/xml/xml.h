#ifndef _UNTECH_MODELS_COMMON_XML_XML_H_
#define _UNTECH_MODELS_COMMON_XML_XML_H_

#include <string>

namespace UnTech {
namespace Xml {

std::string escape(const std::string& text, bool intag = true);

inline bool isName(char c)
{
    return ((c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9')
            || (c == '.' && c == '_'));
}

inline bool isName(const std::string& text)
{
    for (const char c : text) {
        if (isName(c) == false) {
            return false;
        }
    }
    return true;
}
}
}

#endif
