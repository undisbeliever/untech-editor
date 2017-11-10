/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/metatiles/metatiles-serializer.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace UnTech;
namespace MT = UnTech::MetaTiles;

std::unique_ptr<MT::MetaTileTilesetInput> readXmlString(const std::string& str)
{
    Xml::XmlReader xml(str, "STRING");
    return MT::readMetaTileTilesetInput(xml);
}

std::string writeXmlString(const MT::MetaTileTilesetInput& mti)
{
    std::stringstream stream;
    Xml::XmlWriter xml(stream, "STRING", "untech");
    MT::writeMetaTileTilesetInput(xml, mti);

    return stream.str();
}

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cerr << "ERROR: expected argument" << std::endl;
        return EXIT_FAILURE;
    }

    std::string filename = argv[1];
    std::unique_ptr<MT::MetaTileTilesetInput> mti1;
    std::unique_ptr<MT::MetaTileTilesetInput> mti2;

    try {
        mti1 = MT::loadMetaTileTilesetInput(filename);
    }
    catch (const Xml::xml_error& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::runtime_error& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::string xml1 = writeXmlString(*mti1);
    mti2 = readXmlString(xml1);

    if (*mti1 != *mti2) {
        std::cerr << "ERROR: mti1 != mti2" << std::endl;
        abort();
    }

    std::string xml2 = writeXmlString(*mti2);

    if (xml1 != xml2) {
        std::cerr << "ERROR: Mismatched xml" << std::endl;
        abort();
    }

    return EXIT_SUCCESS;
}
