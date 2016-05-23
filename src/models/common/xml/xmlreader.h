#pragma once

#include "xml.h"
#include "../aabb.h"
#include "../string.h"
#include <cstdint>
#include <memory>
#include <sstream>
#include <stack>
#include <string>
#include <vector>

namespace UnTech {
namespace Xml {

struct XmlTag;

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
 *
 * The parser functions can raise a std::runtime_error with a human readable
 * description of the parsing error and its location.
 */
class XmlReader {

public:
    XmlReader(const std::string& xml, const std::string& filename = "");

    static std::unique_ptr<XmlReader> fromFile(const std::string& filename);

    /** restart processing from the beginning */
    void parseDocument();

    /**
     * This method skips text/whitespace before the next tag.
     * returns a nullptr if there are no tags in the current level.
     *
     * MEMORY SAFETY: The XmlTag relies on the XmlReader.
     * It must not exist when this XmlReader is reclaimed.
     */
    std::unique_ptr<XmlTag> parseTag();

    /** returns the text at the current cursor */
    std::string parseText();

    /** returns the base64 data at the current cursor */
    std::vector<uint8_t> parseBase64();

    /** This method will skip over any child/sibling text/tags in order to close the current tag */
    void parseCloseTag();

    /** the current line number of the cursor */
    inline unsigned lineNo() const { return _lineNo; }

    /** The filename of the XML file, may be empty */
    inline std::string filename() const { return _filename; }

    /** The directory part of the filename.
     * Is either empty of ends in a slash. */
    inline std::string dirname() const { return _dirname; }

    /** the file part of the filename. */
    inline std::string filepart() const { return _filepart; }

    /**
     * builds a std::runtime_error with the filename and line number of
     * prepending the given message.
     */
    std::runtime_error buildError(const char* msg) const
    {
        std::stringstream stream;

        auto fp = _filepart;
        if (fp.empty()) {
            fp = "XML";
        }

        stream << fp << ":" << _lineNo << ": " << msg;
        return std::runtime_error(stream.str());
    }

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
    std::string _filename;
    std::string _filepart;
    std::string _dirname;
};
}
}
#include "xmltag.h"
