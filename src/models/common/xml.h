#ifndef _UNTECH_MODELS_COMMON_XML_H_
#define _UNTECH_MODELS_COMMON_XML_H_

#include "aabb.h"
#include "string.h"
#include "xml/xmltag.h"
#include <memory>
#include <stack>
#include <string>

namespace UnTech {
namespace Xml {

std::string escape(const std::string& text, bool intag = true);

/**
 * The `XmlReader` class allows for the parsing of a subset of the XML spec.
 *
 * It only supports:
 *      * UTF8 text encoding
 *      * tags/attributes (without namespaces)
 *      * comments
 *      * cdata
 *      * &lt;, &gt;, &amp;, &apos; and &quot; escape sequences
 *
 * It is inspired by Qt's QXmlStreamReader, but is simpler and will cause an
 * exception if it tries to process malformed XML.
 */
class XmlReader {

public:
    XmlReader(const std::string& xml);

    /** restart processing from the beginning */
    void parseDocument();

    /**
     * This method skips text/whitespace before the next tag
     * returns a nullptr if there are no tags in the current level
     */
    std::unique_ptr<XmlTag> parseTag();

    /** returns the text at the current cursor */
    std::string parseText();

    /** This method will skip over any child/sibling text/tags in order to close the current tag */
    void parseCloseTag();

    /** the current line number of the cursor */
    inline unsigned lineNo() const { return _lineNo; }

private:
    void skipWhitespace();
    void skipText();
    std::string parseName();
    std::string parseAttributeValue();

private:
    const std::string _inputString;
    const char* _pos;
    std::stack<std::string> _tagStack;
    bool _inSelfClosingTag;
    unsigned _lineNo;
};
}
}

#endif
