/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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
#include "models/resources/scenes.h"
#include "models/rooms/rooms.h"
#include "models/scripting/bytecode.h"
#include "models/scripting/game-state.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech::Project {

struct ProjectSettings {
    MemoryMapSettings memoryMap;
    Rooms::RoomSettings roomSettings;

    bool operator==(const ProjectSettings&) const = default;
};

bool validateProjectSettings(const ProjectSettings& input, ErrorList& err);

struct ProjectFile {
    const static std::string FILE_EXTENSION;

    ProjectSettings projectSettings;

    Scripting::GameState gameState;
    Scripting::BytecodeInput bytecode;

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

    bool operator==(const ProjectFile&) const = default;
};

std::unique_ptr<ProjectFile> loadProjectFile(const std::filesystem::path& filename);
void saveProjectFile(const ProjectFile& project, const std::filesystem::path& filename);

}
