/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "models/common/file.h"
#include "models/common/u8strings.h"
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
#include "models/rooms/rooms-serializer.h"
#include <cstdlib>
#include <iostream>

using namespace UnTech;

static size_t nFilesPassed = 0;
static size_t nFilesFailed = 0;

template <class T, typename Reader, typename Writer>
static void validateReaderAndWriter(const std::filesystem::path& filename, const T& input,
                                    Reader readerFunction, Writer writerFunction)
{
    // Large buffer size: Some of these XML files contain a large base64 text block
    Xml::XmlWriter xmlWriter_1(filename, u8"untech", 128 * 1024);
    writerFunction(xmlWriter_1, input);

    std::unique_ptr<T> output;
    try {
        Xml::XmlReader xmlReader(std::u8string(xmlWriter_1.string_view()), filename);
        output = readerFunction(xmlReader);
    }
    catch (const std::exception& ex) {
        throw runtime_error(u8"Unable to read output of : XmlWriter", convert_old_string(ex.what()));
    }

    assert(output);
    if ((*output == input) == false) {
        throw runtime_error(filename.u8string(), u8": output != input");
    }

    // Small buffer size: test StringBuilder will increase buffer size properly
    Xml::XmlWriter xmlWriter_2(filename, u8"untech", 1024);
    writerFunction(xmlWriter_2, input);

    if (xmlWriter_1.string_view() != xmlWriter_2.string_view()) {
        throw runtime_error(filename.u8string(), u8": xmlWriter_1 output != xmlWriter_2 output");
    }
}

template <typename Reader, typename Writer>
static bool testSerializer(const std::filesystem::path& filename, Reader reader, Writer writer)
{
    try {
        auto xml = Xml::XmlReader::fromFile(filename);
        assert(xml);
        auto data = reader(*xml);

        assert(data != nullptr);
        validateReaderAndWriter(filename, *data, reader, writer);

        nFilesPassed++;
        stdout_write(data->FILE_EXTENSION);
        std::cout << " serializer passed: " << filename << std::endl;
        return true;
    }
    catch (const std::exception& ex) {
        nFilesFailed++;
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return false;
    }
}

template <class T>
static bool testSerializer(const std::filesystem::path& filename);

template <>
bool testSerializer<MetaTiles::MetaTileTilesetInput>(const std::filesystem::path& filename)
{
    return testSerializer(filename, MetaTiles::readMetaTileTilesetInput, MetaTiles::writeMetaTileTilesetInput);
}

template <>
bool testSerializer<MetaSprite::FrameSetExportOrder>(const std::filesystem::path& filename)
{
    return testSerializer(filename, MetaSprite::readFrameSetExportOrder, MetaSprite::writeFrameSetExportOrder);
}

template <>
bool testSerializer<Rooms::RoomInput>(const std::filesystem::path& filename)
{
    return testSerializer(filename, Rooms::readRoomInput, Rooms::writeRoomInput);
}

template <>
bool testSerializer<MetaSprite::MetaSprite::FrameSet>(const std::filesystem::path& filename)
{
    return testSerializer(filename, MetaSprite::MetaSprite::readFrameSet, MetaSprite::MetaSprite::writeFrameSet);
}

template <>
bool testSerializer<MetaSprite::SpriteImporter::FrameSet>(const std::filesystem::path& filename)
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
    std::cerr << "ERROR: Unknown FrameSetFile type: " << f.filename.string();
    return false;
}

static bool testFrameSetFiles(const std::vector<MetaSprite::FrameSetFile>& frameSets)
{
    bool valid = true;
    for (const auto& f : frameSets) {
        // cppcheck-suppress useStlAlgorithm
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
bool testSerializer<Project::ProjectFile>(const std::filesystem::path& filename)
{
    std::unique_ptr<Project::ProjectFile> project;

    try {
        project = Project::loadProjectFile(filename);
        assert(project);
        validateReaderAndWriter(filename, *project, Project::readProjectFile, Project::writeProjectFile);

        nFilesPassed++;
        stdout_write(project->FILE_EXTENSION);
        std::cout << " serializer passed: " << filename << std::endl;
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
    valid &= testExternalFileList(project->rooms);

    return valid;
}

static bool testFilenameOfUnknownType(const std::filesystem::path& filePath)
{
    const auto extension = filePath.extension();

    std::u8string extWithoutDot = filePath.extension().u8string();
    if (not extWithoutDot.empty() and extWithoutDot[0] == u8'.') {
        extWithoutDot.erase(0, 1);
    }

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define TEST(CLASS)                                   \
    {                                                 \
        if (extWithoutDot == CLASS::FILE_EXTENSION) { \
            return testSerializer<CLASS>(filePath);   \
        }                                             \
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
