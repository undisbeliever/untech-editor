/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-serializer.h"
#include "project.h"
#include "models/entity/entityromdata-serializer.h"
#include "models/metasprite/actionpointfunctions-serializer.h"
#include "models/metasprite/framesetfile-serializer.h"
#include "models/metatiles/metatiles-serializer.h"
#include "models/resources/resources-serializer.h"
#include "models/rooms/rooms-serializer.h"
#include "models/scripting/scripting-serializer.h"

using namespace UnTech::Xml;

namespace UnTech::Project {

const std::u8string ProjectFile::FILE_EXTENSION = u8"utproject";

using FrameSetFile = UnTech::MetaSprite::FrameSetFile;

static const EnumMap<MappingMode> mappingModeEnumMap = {
    { u8"lorom", MappingMode::LOROM },
    { u8"hirom", MappingMode::HIROM },
};

static void readMemoryMapSettings(const XmlTag& tag, MemoryMapSettings& mmap)
{
    assert(tag.name == u8"memory-map");

    mmap.mode = tag.getAttributeEnum(u8"mode", mappingModeEnumMap);
    mmap.firstBank = tag.getAttributeUnsignedHex(u8"first-bank");
    mmap.nBanks = tag.getAttributeUnsigned(u8"n-banks");
}

static void writeMemoryMapSettings(XmlWriter& xml, const MemoryMapSettings& mmap)
{
    xml.writeTag(u8"memory-map");
    xml.writeTagAttributeEnum(u8"mode", mmap.mode, mappingModeEnumMap);
    xml.writeTagAttributeHex(u8"first-bank", mmap.firstBank);
    xml.writeTagAttribute(u8"n-banks", mmap.nBanks);
    xml.writeCloseTag();
}

template <class T>
static void readExternalFileList(const XmlTag& tag, ExternalFileList<T>& list)
{
    list.insert_back(tag.getAttributeFilename(u8"src"));
}

template <class T>
static void writeExternalFileList(XmlWriter& xml, const std::u8string& tagName,
                                  const ExternalFileList<T>& list)
{
    for (const auto& it : list) {
        xml.writeTag(tagName);
        xml.writeTagAttributeFilename(u8"src", it.filename);
        xml.writeCloseTag();
    }
}

std::unique_ptr<ProjectFile> readProjectFile(XmlReader& xml)
{
    const auto tag = xml.parseTag();
    if (tag.name != u8"project") {
        throw xml_error(xml, u8"Not an untech-editor project (expected <project>");
    }

    auto project = std::make_unique<ProjectFile>();

    bool readMemoryMapTag = false;
    bool readGameStateTag = false;
    bool readBytecodeTag = false;
    bool readRoomSettingsTag = false;

    while (const auto childTag = xml.parseTag()) {
        if (childTag.name == u8"exportorder") {
            readExternalFileList(childTag, project->frameSetExportOrders);
        }
        else if (childTag.name == u8"frameset") {
            MetaSprite::readFrameSetFile(childTag, project->frameSets);
        }
        else if (childTag.name == u8"palette") {
            Resources::readPalette(childTag, project->palettes);
        }
        else if (childTag.name == u8"background-image") {
            Resources::readBackgroundImage(childTag, project->backgroundImages);
        }
        else if (childTag.name == u8"metatile-tileset") {
            readExternalFileList(childTag, project->metaTileTilesets);
        }
        else if (childTag.name == u8"room-file") {
            readExternalFileList(childTag, project->rooms);
        }
        else if (childTag.name == u8"entity-rom-data") {
            Entity::readEntityRomData(xml, childTag, project->entityRomData);
        }
        else if (childTag.name == u8"action-point-function") {
            MetaSprite::readActionPointFunction(childTag, project->actionPointFunctions);
        }
        else if (childTag.name == u8"scene-setting") {
            Resources::readSceneSetting(childTag, project->resourceScenes.settings);
        }
        else if (childTag.name == u8"scene") {
            Resources::readScene(childTag, project->resourceScenes.scenes);
        }
        else if (childTag.name == u8"memory-map") {
            if (readMemoryMapTag) {
                throw xml_error(childTag, u8"Only one <memory-map> tag is allowed");
            }
            readMemoryMapSettings(childTag, project->projectSettings.memoryMap);
            readMemoryMapTag = true;
        }
        else if (childTag.name == u8"block-settings") {
            // block-settings has been removed, skip
        }
        else if (childTag.name == u8"metatile-engine-settings") {
            // metatile-engine-settings has been removed, skip
        }
        else if (childTag.name == u8"game-state") {
            if (readGameStateTag) {
                throw xml_error(childTag, u8"Only one <game-state> tag is allowed");
            }
            readGameStateTag = true;

            Scripting::readGameState(project->gameState, xml, childTag);
        }
        else if (childTag.name == u8"bytecode") {
            if (readBytecodeTag) {
                throw xml_error(childTag, u8"Only one <bytecode> tag is allowed");
            }
            readBytecodeTag = true;

            Scripting::readBytecode(project->bytecode, xml, childTag);
        }
        else if (childTag.name == u8"interactive-tiles") {
            MetaTiles::readInteractiveTiles(xml, childTag, project->interactiveTiles);
        }
        else if (childTag.name == u8"room-settings") {
            if (readRoomSettingsTag) {
                throw xml_error(childTag, u8"Only one <room-settings> tag is allowed");
            }
            readRoomSettingsTag = true;

            Rooms::readRoomSettings(project->projectSettings.roomSettings, childTag);
        }
        else {
            throw unknown_tag_error(childTag);
        }

        xml.parseCloseTag();
    }

    return project;
}

void writeProjectFile(XmlWriter& xml, const ProjectFile& project)
{
    xml.writeTag(u8"project");

    Project::writeMemoryMapSettings(xml, project.projectSettings.memoryMap);
    Rooms::writeRoomSettings(xml, project.projectSettings.roomSettings);

    Scripting::writeGameState(xml, project.gameState);
    Scripting::writeBytecode(xml, project.bytecode);

    MetaTiles::writeInteractiveTiles(xml, project.interactiveTiles);

    Entity::writeEntityRomData(xml, project.entityRomData);

    MetaSprite::writeActionPointFunctions(xml, project.actionPointFunctions);
    writeExternalFileList(xml, u8"exportorder", project.frameSetExportOrders);
    MetaSprite::writeFrameSetFiles(xml, project.frameSets);
    Resources::writePalettes(xml, project.palettes);
    Resources::writeBackgroundImages(xml, project.backgroundImages);
    writeExternalFileList(xml, u8"metatile-tileset", project.metaTileTilesets);
    writeExternalFileList(xml, u8"room-file", project.rooms);

    Resources::writeSceneSettings(xml, project.resourceScenes.settings);
    Resources::writeScenes(xml, project.resourceScenes.scenes);

    xml.writeCloseTag();
}

/*
 * API
 * ===
 */

std::unique_ptr<ProjectFile> loadProjectFile(const std::filesystem::path& filename)
{
    auto xml = XmlReader::fromFile(filename);
    try {
        assert(xml);
        return readProjectFile(*xml);
    }
    catch (const std::exception& ex) {
        throw xml_error(*xml, u8"Error loading untech-editor project file", ex);
    }
}

void saveProjectFile(const ProjectFile& project, const std::filesystem::path& filename)
{
    // utproject files are large, use a larger buffer.
    XmlWriter xml(filename, u8"untech", 64 * 1024);
    writeProjectFile(xml, project);

    File::atomicWrite(filename, xml.string_view());
}

}
