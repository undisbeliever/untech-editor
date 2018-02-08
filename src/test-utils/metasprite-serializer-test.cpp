/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/metasprite/metasprite-serializer.h"
#include "models/metasprite/metasprite.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace UnTech;
namespace MS = UnTech::MetaSprite::MetaSprite;

std::unique_ptr<MS::FrameSet> readXmlString(const std::string& str)
{
    Xml::XmlReader xml(str, "STRING");
    return MS::readFrameSet(xml);
}

std::string writeXmlString(const MS::FrameSet& frameSet)
{
    std::stringstream stream;
    Xml::XmlWriter xml(stream, "STRING", "untech");
    writeFrameSet(xml, frameSet);

    return stream.str();
}

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cerr << "ERROR: expected argument" << std::endl;
        return EXIT_FAILURE;
    }

    std::string filename = argv[1];
    std::unique_ptr<MS::FrameSet> frameSet1;
    std::unique_ptr<MS::FrameSet> frameSet2;

    try {
        frameSet1 = MS::loadFrameSet(filename);
    }
    catch (const Xml::xml_error& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::runtime_error& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::string xml1 = writeXmlString(*frameSet1);
    frameSet2 = readXmlString(xml1);

    if (*frameSet1 != *frameSet2) {
        std::cerr << "ERROR: frameSet1 != frameSet2" << std::endl;
        abort();
    }

    std::string xml2 = writeXmlString(*frameSet2);

    if (xml1 != xml2) {
        std::cerr << "ERROR: Mismatched xml" << std::endl;
        abort();
    }

    return EXIT_SUCCESS;
}
