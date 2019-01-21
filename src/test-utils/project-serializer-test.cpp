/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/project/project-serializer.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace UnTech;
using namespace Project;

std::unique_ptr<ProjectFile> readXmlString(const std::string& str)
{
    Xml::XmlReader xml(str, "STRING");
    return readProjectFile(xml);
}

std::string writeXmlString(const ProjectFile& resFile)
{
    std::stringstream stream;
    Xml::XmlWriter xml(stream, "STRING", "untech");
    writeProjectFile(xml, resFile);

    return stream.str();
}

int main(int argc, const char* argv[])
{
    if (argc != 2) {
        std::cerr << "ERROR: expected argument" << std::endl;
        return EXIT_FAILURE;
    }

    std::string filename = argv[1];
    std::unique_ptr<ProjectFile> resFile1;
    std::unique_ptr<ProjectFile> resFile2;

    try {
        resFile1 = loadProjectFile(filename);
    }
    catch (const Xml::xml_error& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::runtime_error& ex) {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }

    std::string xml1 = writeXmlString(*resFile1);
    resFile2 = readXmlString(xml1);

    if (*resFile1 != *resFile2) {
        std::cerr << "ERROR: resFile1 != resFile2" << std::endl;
        abort();
    }

    std::string xml2 = writeXmlString(*resFile2);

    if (xml1 != xml2) {
        std::cerr << "ERROR: Mismatched xml" << std::endl;
        abort();
    }

    return EXIT_SUCCESS;
}
