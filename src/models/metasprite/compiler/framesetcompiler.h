/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framesetexportlist.h"
#include "tilesetinserter.h"
#include "models/metasprite/framesetfile.h"
#include <cstdint>
#include <vector>

namespace UnTech {
namespace MetaSprite {
namespace Compiler {

struct TileHitboxData {
    uint8_t left;
    uint8_t right;
    uint8_t yOffset;
    uint8_t height;
};

struct FrameData {
    std::vector<uint8_t> frameObjects;
    std::vector<uint8_t> entityHitboxes;
    std::vector<uint8_t> actionPoints;

    // Index into `FrameSetData::tileset`
    std::optional<unsigned> tileset;
    TileHitboxData tileHitbox;
};

struct FrameSetData {
    TilesetData tileset;

    std::vector<FrameData> frames;
    std::vector<std::vector<uint8_t>> animations;
    std::vector<std::array<uint8_t, 30>> palettes;

    // Only set on SpriteImporter FrameSets
    std::unique_ptr<const UnTech::MetaSprite::MetaSprite::FrameSet> msFrameSet;

    // Required for ProjectData::DataStore
    bool validate(ErrorList&) const { return true; }
};

std::shared_ptr<const FrameSetData>
compileFrameSet(const UnTech::MetaSprite::FrameSetFile& fs,
                const Project::ProjectFile& project, const ActionPointMapping& actionPointMapping,
                ErrorList& errorList);
}
}
}
