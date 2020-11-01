/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "memorymap.h"
#include "models/common/externalfilelist.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include "models/entity/entityromdata.h"
#include "models/metasprite/frameset-exportorder.h"
#include "models/metasprite/framesetfile.h"
#include "models/metatiles/common.h"
#include "models/metatiles/interactive-tiles.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/resources/background-image.h"
#include "models/resources/palette.h"
#include "models/resources/scenes.h"
#include "models/rooms/rooms.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace Project {

struct ProjectSettings {
    MemoryMapSettings memoryMap;
    Rooms::RoomSettings roomSettings;

    bool validate(ErrorList& err) const;

    bool operator==(const ProjectSettings& o) const
    {
        return memoryMap == o.memoryMap
               && roomSettings == o.roomSettings;
    }
};

struct ProjectFile {
    const static std::string FILE_EXTENSION;

    ProjectSettings projectSettings;

    MetaTiles::InteractiveTiles interactiveTiles;
    Entity::EntityRomData entityRomData;
    Resources::ResourceScenes resourceScenes;

    NamedList<Resources::PaletteInput> palettes;
    NamedList<Resources::BackgroundImageInput> backgroundImages;
    ExternalFileList<MetaTiles::MetaTileTilesetInput> metaTileTilesets;

    NamedList<MetaSprite::ActionPointFunction> actionPointFunctions;
    std::vector<MetaSprite::FrameSetFile> frameSets;
    ExternalFileList<MetaSprite::FrameSetExportOrder> frameSetExportOrders;

    ExternalFileList<Rooms::RoomInput> rooms;

    void loadAllFiles();

    void loadAllFilesIgnoringErrors();

    bool validate(ErrorList& err) const;

    bool operator==(const ProjectFile& o) const
    {
        return projectSettings == o.projectSettings
               && interactiveTiles == o.interactiveTiles
               && entityRomData == o.entityRomData
               && resourceScenes == o.resourceScenes
               && palettes == o.palettes
               && backgroundImages == o.backgroundImages
               && metaTileTilesets == o.metaTileTilesets
               && actionPointFunctions == o.actionPointFunctions
               && frameSets == o.frameSets
               && frameSetExportOrders == o.frameSetExportOrders
               && rooms == o.rooms;
    }
    bool operator!=(const ProjectFile& o) const { return !(*this == o); }
};

std::unique_ptr<ProjectFile> loadProjectFile(const std::filesystem::path& filename);
void saveProjectFile(const ProjectFile& project, const std::filesystem::path& filename);

}
}
