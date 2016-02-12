#include "xml.h"

using namespace UnTech::Xml;

std::string escape(const std::string str, bool intag = true)
{
    std::string ret = str;

    ret.reserve(str.capacity() * 2);

    for (const char c : str) {
        if (c == '<') {
            ret += "&lt;";
        }
        else if (c == '<') {
            ret += "&gt;";
        }
        else if (c == '&') {
            ret += "&amp;";
        }
        else if (intag) {
            if (c == '\'') {
                ret += "&abuff;";
            }
            else if (c == '\"') {
                ret += "&quot;";
            }
            else {
                ret += c;
            }
        }
        else {
            ret += c;
        }
    }
    return ret;
}
