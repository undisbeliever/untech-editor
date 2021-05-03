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

namespace UnTech::MetaSprite::Compiler {

struct FrameData {
    std::vector<uint8_t> frameObjects;
    std::vector<uint8_t> actionPoints;
    std::array<uint8_t, 16> collisionBoxes;

    // Index into `FrameSetData::tileset`
    std::optional<unsigned> tileset;
};

struct FrameSetData {
    TilesetData tileset;

    std::vector<FrameData> frames;
    std::vector<std::vector<uint8_t>> animations;
    std::vector<std::array<uint8_t, 30>> palettes;

    // Only set on SpriteImporter FrameSets
    std::unique_ptr<const UnTech::MetaSprite::MetaSprite::FrameSet> msFrameSet;
};

std::shared_ptr<const FrameSetData>
compileFrameSet(const UnTech::MetaSprite::FrameSetFile& fs,
                const Project::ProjectFile& project, const ActionPointMapping& actionPointMapping,
                ErrorList& errorList);

}
