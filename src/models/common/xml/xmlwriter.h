#ifndef _UNTECH_MODELS_COMMON_XML_XMLWRITER_H_
#define _UNTECH_MODELS_COMMON_XML_XMLWRITER_H_

#include "xml.h"
#include "../aabb.h"
#include "../ms8aabb.h"
#include "../string.h"
#include <stack>
#include <string>
#include <ostream>

namespace UnTech {
namespace Xml {

/**
 * The `XmlWriter` class simplifies the creation of XML documents.
 */
class XmlWriter {

public:
    XmlWriter(std::ostream& output, const std::string& doctype)
        : XmlWriter(output, "", doctype)
    {
    }
    XmlWriter(std::ostream& output, const std::string& filename, const std::string& doctype);
    ~XmlWriter();

    const std::string& filename() const { return _filename; }
    const std::string& dirname() const { return _dirname; }

    void writeTag(const std::string& name);

    void writeTagAttribute(const std::string& name, const std::string& value);
    void writeTagAttribute(const std::string& name, const char* value);
    void writeTagAttribute(const std::string& name, const int value);
    void writeTagAttribute(const std::string& name, const unsigned value);
    void writeTagAttributeHex(const std::string& name, const unsigned value, unsigned width);

    void writeText(const std::string& text);

    void writeCloseTag();

    void writeTagAttributeFilename(const std::string& name, const std::string& filename);

    inline void writeTagAttribute(const std::string& name, bool v)
    {
        if (v) {
            writeTagAttribute(name, "true");
        }
    }

    inline void writeTagAttributeUpoint(const upoint& p, const std::string& xName = "x", const std::string& yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeUsize(const usize& s, const std::string& widthName = "width", const std::string& heightName = "height")
    {
        writeTagAttribute(widthName, s.width);
        writeTagAttribute(heightName, s.height);
    }

    inline void writeTagAttributeUrect(const urect& r, const std::string& xName = "x", const std::string& yName = "y", const std::string& widthName = "width", const std::string& heightName = "height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

    inline void writeTagAttributeMs8point(const ms8point& p, const std::string& xName = "x", const std::string& yName = "y")
    {
        writeTagAttribute(xName, p.x);
        writeTagAttribute(yName, p.y);
    }

    inline void writeTagAttributeMs8rect(const ms8rect& r, const std::string& xName = "x", const std::string& yName = "y", const std::string& widthName = "width", const std::string& heightName = "height")
    {
        writeTagAttribute(xName, r.x);
        writeTagAttribute(yName, r.y);
        writeTagAttribute(widthName, r.width);
        writeTagAttribute(heightName, r.height);
    }

private:
    void writeCloseTagHead();
    void writeEscapeAttribute(const std::string& text);
    void writeEscapeAttribute(const char* text);

private:
    std::ostream& _file;
    std::stack<std::string> _tagStack;
    std::string _filename;
    std::string _dirname;
    bool _inTag;
};
}
}

#endif
