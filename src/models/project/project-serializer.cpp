/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project-serializer.h"
#include "project.h"
#include "models/common/atomicofstream.h"
#include "models/entity/entityromdata-serializer.h"
#include "models/metasprite/actionpointfunctions-serializer.h"
#include "models/metasprite/framesetfile-serializer.h"
#include "models/resources/resources-serializer.h"
#include "models/rooms/rooms-serializer.h"

using namespace UnTech;
using namespace UnTech::Xml;

namespace UnTech {
namespace Project {

const std::string ProjectFile::FILE_EXTENSION = "utproject";

using FrameSetFile = UnTech::MetaSprite::FrameSetFile;

static void readBlockSettings(const XmlTag* tag, BlockSettings& settings)
{
    assert(tag->name == "block-settings");

    settings.size = tag->getAttributeUnsigned("block-size");
    settings.count = tag->getAttributeUnsigned("block-count");
}

static void writeBlockSettings(XmlWriter& xml, const BlockSettings& settings)
{
    xml.writeTag("block-settings");
    xml.writeTagAttribute("block-size", settings.size);
    xml.writeTagAttribute("block-count", settings.count);
    xml.writeCloseTag();
}

template <class T>
static void readExternalFileList(const XmlTag* tag, ExternalFileList<T>& list)
{
    list.insert_back(tag->getAttributeFilename("src"));
}

template <class T>
static void writeExternalFileList(XmlWriter& xml, const std::string& tagName,
                                  const ExternalFileList<T>& list)
{
    for (const auto& it : list) {
        xml.writeTag(tagName);
        xml.writeTagAttributeFilename("src", it.filename);
        xml.writeCloseTag();
    }
}

std::unique_ptr<ProjectFile> readProjectFile(XmlReader& xml)
{
    std::unique_ptr<XmlTag> tag = xml.parseTag();
    if (tag == nullptr || tag->name != "project") {
        throw xml_error(xml, "Not an untech-editor project (expected <project>");
    }

    auto project = std::make_unique<ProjectFile>();

    std::unique_ptr<XmlTag> childTag;

    bool readBlockSettingsTag = false;
    bool readRoomSettingsTag = false;

    while ((childTag = xml.parseTag())) {
        if (childTag->name == "exportorder") {
            readExternalFileList(childTag.get(), project->frameSetExportOrders);
        }
        else if (childTag->name == "frameset") {
            MetaSprite::readFrameSetFile(childTag.get(), project->frameSets);
        }
        else if (childTag->name == "palette") {
            Resources::readPalette(childTag.get(), project->palettes);
        }
        else if (childTag->name == "background-image") {
            Resources::readBackgroundImage(childTag.get(), project->backgroundImages);
        }
        else if (childTag->name == "metatile-tileset") {
            readExternalFileList(childTag.get(), project->metaTileTilesets);
        }
        else if (childTag->name == "room-file") {
            readExternalFileList(childTag.get(), project->rooms);
        }
        else if (childTag->name == "entity-rom-data") {
            Entity::readEntityRomData(xml, childTag.get(), project->entityRomData);
        }
        else if (childTag->name == "action-point-function") {
            MetaSprite::readActionPointFunction(childTag.get(), project->actionPointFunctions);
        }
        else if (childTag->name == "scene-setting") {
            Resources::readSceneSetting(childTag.get(), project->resourceScenes.settings);
        }
        else if (childTag->name == "scene") {
            Resources::readScene(childTag.get(), project->resourceScenes.scenes);
        }
        else if (childTag->name == "block-settings") {
            if (readBlockSettingsTag) {
                throw xml_error(*childTag, "Only one <block-settings> tag is allowed");
            }
            readBlockSettings(childTag.get(), project->blockSettings);
            readBlockSettingsTag = true;
        }
        else if (childTag->name == "metatile-engine-settings") {
            // metatile-engine-settings has been removed, skip
        }
        else if (childTag->name == "room-settings") {
            if (readRoomSettingsTag) {
                throw xml_error(*childTag, "Only one <room-settings> tag is allowed");
            }
            readRoomSettingsTag = true;

            Rooms::readRoomSettings(project->roomSettings, childTag.get());
        }
        else {
            throw unknown_tag_error(*childTag);
        }

        xml.parseCloseTag();
    }

    return project;
}

void writeProjectFile(XmlWriter& xml, const ProjectFile& project)
{
    xml.writeTag("project");

    Project::writeBlockSettings(xml, project.blockSettings);
    Rooms::writeRoomSettings(xml, project.roomSettings);

    Entity::writeEntityRomData(xml, project.entityRomData);

    MetaSprite::writeActionPointFunctions(xml, project.actionPointFunctions);
    writeExternalFileList(xml, "exportorder", project.frameSetExportOrders);
    MetaSprite::writeFrameSetFiles(xml, project.frameSets);
    Resources::writePalettes(xml, project.palettes);
    Resources::writeBackgroundImages(xml, project.backgroundImages);
    writeExternalFileList(xml, "metatile-tileset", project.metaTileTilesets);
    writeExternalFileList(xml, "room-file", project.rooms);

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
        throw xml_error(*xml, "Error loading untech-editor project file", ex);
    }
}

void saveProjectFile(const ProjectFile& project, const std::filesystem::path& filename)
{
    AtomicOfStream file(filename);
    XmlWriter xml(file, filename, "untech");
    writeProjectFile(xml, project);
    file.commit();
}
}
}
