/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/externalfilelist.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include "models/entity/entityromdata.h"
#include "models/metasprite/frameset-exportorder.h"
#include "models/metasprite/framesetfile.h"
#include "models/metatiles/common.h"
#include "models/metatiles/metatile-tileset.h"
#include "models/resources/background-image.h"
#include "models/resources/palette.h"
#include "models/resources/scenes.h"
#include <memory>
#include <string>
#include <vector>

namespace UnTech {
namespace Project {

struct BlockSettings {
    unsigned size = 8192;
    unsigned count = 1;

    bool validate(ErrorList& err) const;

    bool operator==(const BlockSettings& o) const
    {
        return size == o.size
               && count == o.count;
    }
    bool operator!=(const BlockSettings& o) const { return !(*this == o); }
};

struct ProjectFile {
    const static std::string FILE_EXTENSION;

    BlockSettings blockSettings;
    MetaTiles::EngineSettings metaTileEngineSettings;

    Entity::EntityRomData entityRomData;
    Resources::ResourceScenes resourceScenes;

    NamedList<Resources::PaletteInput> palettes;
    NamedList<Resources::BackgroundImageInput> backgroundImages;
    ExternalFileList<MetaTiles::MetaTileTilesetInput> metaTileTilesets;

    NamedList<MetaSprite::ActionPointFunction> actionPointFunctions;
    std::vector<MetaSprite::FrameSetFile> frameSets;
    ExternalFileList<MetaSprite::FrameSetExportOrder> frameSetExportOrders;

    void loadAllFiles();

    bool validate(ErrorList& err) const;

    bool operator==(const ProjectFile& o) const
    {
        return blockSettings == o.blockSettings
               && metaTileEngineSettings == o.metaTileEngineSettings
               && entityRomData == o.entityRomData
               && resourceScenes == o.resourceScenes
               && palettes == o.palettes
               && backgroundImages == o.backgroundImages
               && metaTileTilesets == o.metaTileTilesets
               && actionPointFunctions == o.actionPointFunctions
               && frameSets == o.frameSets
               && frameSetExportOrders == o.frameSetExportOrders;
    }
    bool operator!=(const ProjectFile& o) const { return !(*this == o); }
};

std::unique_ptr<ProjectFile> loadProjectFile(const std::filesystem::path& filename);
void saveProjectFile(const ProjectFile& project, const std::filesystem::path& filename);

}
}
