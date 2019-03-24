/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "models/common/file.h"
#include "models/common/xml/xmlreader.h"
#include "models/common/xml/xmlwriter.h"
#include "models/entity/entityromdata-serializer.h"
#include "models/metasprite/frameset-exportorder-serializer.h"
#include "models/metasprite/framesetfile-serializer.h"
#include "models/metasprite/metasprite-serializer.h"
#include "models/metasprite/spriteimporter-serializer.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/project/project-serializer.h"
#include "models/resources/resources-serializer.h"
#include <cstdlib>
#include <iostream>
#include <sstream>

using namespace UnTech;

static size_t nFilesPassed = 0;
static size_t nFilesFailed = 0;

template <class T, typename Writer>
static std::string writeXmlString(const T& input, const std::string& filename, Writer writer)
{
    std::stringstream stream;
    Xml::XmlWriter xml(stream, filename, "untech");
    writer(xml, input);

    return stream.str();
}

template <class T, typename Reader, typename Writer>
static void validateReaderAndWriter(const std::string& filename, const T& input,
                                    Reader readerFunction, Writer writerFunction)
{
    const std::string xmlString1 = writeXmlString(input, filename, writerFunction);

    std::unique_ptr<T> output;
    try {
        Xml::XmlReader xml(xmlString1, filename);
        output = readerFunction(xml);
    }
    catch (const std::exception& ex) {
        throw std::runtime_error(std::string("Unable to read xmlString1: ") + ex.what());
    }

    assert(output);
    if ((*output == input) == false) {
        throw std::runtime_error(filename + ": output != input");
    }

    const std::string xmlString2 = writeXmlString(*output, filename, writerFunction);

    if (xmlString1 != xmlString2) {
        throw std::runtime_error(filename + ": xmlString1 != xmlString2");
    }
}

template <typename Reader, typename Writer>
static bool testSerializer(const std::string& filename, Reader reader, Writer writer)
{
    try {
        auto xml = Xml::XmlReader::fromFile(filename);
        assert(xml);
        auto data = reader(*xml);

        assert(data != nullptr);
        validateReaderAndWriter(filename, *data, reader, writer);

        nFilesPassed++;
        std::cout << data->FILE_EXTENSION << " serializer passed: " << filename << std::endl;
        return true;
    }
    catch (const std::exception& ex) {
        nFilesFailed++;
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return false;
    }
}

template <class T>
static bool testSerializer(const std::string& filename);

template <>
bool testSerializer<MetaTiles::MetaTileTilesetInput>(const std::string& filename)
{
    return testSerializer(filename, MetaTiles::readMetaTileTilesetInput, MetaTiles::writeMetaTileTilesetInput);
}

template <>
bool testSerializer<MetaSprite::FrameSetExportOrder>(const std::string& filename)
{
    return testSerializer(filename, MetaSprite::readFrameSetExportOrder, MetaSprite::writeFrameSetExportOrder);
}

template <>
bool testSerializer<MetaSprite::MetaSprite::FrameSet>(const std::string& filename)
{
    return testSerializer(filename, MetaSprite::MetaSprite::readFrameSet, MetaSprite::MetaSprite::writeFrameSet);
}

template <>
bool testSerializer<MetaSprite::SpriteImporter::FrameSet>(const std::string& filename)
{
    return testSerializer(filename, MetaSprite::SpriteImporter::readFrameSet, MetaSprite::SpriteImporter::writeFrameSet);
}

static bool testFrameSetFile(const MetaSprite::FrameSetFile& f)
{
    using FST = MetaSprite::FrameSetFile::FrameSetType;

    switch (f.type) {
    case FST::METASPRITE:
        return testSerializer<MetaSprite::MetaSprite::FrameSet>(f.filename);

    case FST::SPRITE_IMPORTER:
        return testSerializer<MetaSprite::SpriteImporter::FrameSet>(f.filename);

    case FST::UNKNOWN:
        break;
    }

    nFilesFailed++;
    std::cerr << "ERROR: Unknown FrameSetFile type: " + f.filename;
    return false;
}

static bool testFrameSetFiles(const std::vector<MetaSprite::FrameSetFile>& frameSets)
{
    bool valid = true;
    for (auto& f : frameSets) {
        valid &= testFrameSetFile(f);
    }
    return valid;
}

template <class T>
static bool testExternalFileList(const ExternalFileList<T>& list)
{
    bool valid = true;

    for (const ExternalFileItem<T>& item : list) {
        valid &= testSerializer<T>(item.filename);
    }

    return valid;
}

template <>
bool testSerializer<Project::ProjectFile>(const std::string& filename)
{
    std::unique_ptr<Project::ProjectFile> project;

    try {
        project = Project::loadProjectFile(filename);
        assert(project);
        validateReaderAndWriter(filename, *project, Project::readProjectFile, Project::writeProjectFile);

        nFilesPassed++;
        std::cout << project->FILE_EXTENSION << " serializer passed: " << filename << std::endl;
    }
    catch (const std::exception& ex) {
        nFilesFailed++;
        std::cerr << "ERROR: " << ex.what() << " FAILED" << std::endl;
        return false;
    }

    // Also test the external files
    bool valid = true;

    valid &= testExternalFileList(project->metaTileTilesets);
    valid &= testExternalFileList(project->frameSetExportOrders);
    valid &= testFrameSetFiles(project->frameSets);

    return valid;
}

static bool testFilenameOfUnknownType(const std::string& filename)
{
    const std::string extension = File::extension(filename);

#define TEST(CLASS)                             \
    if (extension == CLASS::FILE_EXTENSION) {   \
        return testSerializer<CLASS>(filename); \
    }

    TEST(MetaTiles::MetaTileTilesetInput)
    TEST(MetaSprite::FrameSetExportOrder)
    TEST(MetaSprite::MetaSprite::FrameSet)
    TEST(MetaSprite::SpriteImporter::FrameSet)
    TEST(Project::ProjectFile)

#undef TEST

    nFilesFailed++;
    std::cerr << "ERROR: Unknown extension " << extension << std::endl;
    return false;
}

int main(int argc, const char* argv[])
{
    if (argc <= 1) {
        std::cerr << "ERROR: expected argument" << std::endl;
        return EXIT_FAILURE;
    }

    bool success = true;

    for (int i = 1; i < argc; i++) {
        success &= testFilenameOfUnknownType(argv[i]);
    }

    if (nFilesPassed > 0) {
        std::cout << nFilesPassed << " tests passed" << std::endl;
    }
    if (nFilesFailed > 0) {
        success = false;
        std::cerr << "ERROR " << nFilesFailed << " TESTS FAILED" << std::endl;
    }

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}