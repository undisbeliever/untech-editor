/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include "models/common/unsignedbits.h"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace UnTech::MetaSprite {

const static size_t MAX_FRAMESETS = 1024;
const static size_t MAX_EXPORT_NAMES = 254;
const static size_t MAX_PALETTES = 254;
const static size_t MAX_FRAME_OBJECTS = 32;
const static size_t MAX_ACTION_POINTS = 8;
const static size_t MAX_ANIMATION_FRAMES = 126;

const static size_t MAX_COLLISION_BOX_SIZE = 127;

const static size_t MAX_ACTION_POINT_FUNCTIONS = 126;

const static size_t PALETTE_COLORS = 16;

typedef UnsignedBits<3, uint_fast8_t> SpriteOrderType;
const static SpriteOrderType DEFAULT_SPRITE_ORDER = 2;

enum class TilesetType {
    ONE_TILE_FIXED,
    TWO_TILES_FIXED,
    ONE_ROW_FIXED,
    TWO_ROWS_FIXED,
    ONE_TILE,
    TWO_TILES,
    ONE_ROW,
    TWO_ROWS
};

enum class ObjectSize {
    SMALL,
    LARGE,
};

struct NameReference {
    idstring name;
    bool hFlip = false;
    bool vFlip = false;

    NameReference() = default;

    std::u8string_view flipStringSuffix() const;

    bool operator==(const NameReference&) const = default;
};

struct ActionPointFunction {
    idstring name;

    // If true then the project-compiler will create a define & constant in the
    // "Project.ActionPoints" namespace.
    bool manuallyInvoked = false;

    bool operator==(const ActionPointFunction&) const = default;
};

using ActionPointMapping = std::unordered_map<idstring, uint8_t>;

std::shared_ptr<const ActionPointMapping>
generateActionPointMapping(const NamedList<ActionPointFunction>& apFunctions, ErrorList& err);

}
