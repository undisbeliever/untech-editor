/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once
#include "models/common/clampedinteger.h"
#include "models/common/idstring.h"
#include "models/common/unsignedbits.h"
#include <cstdint>
#include <string>

namespace UnTech {
namespace MetaSprite {

const static size_t MAX_FRAMESETS = 1024;
const static size_t MAX_EXPORT_NAMES = 254;
const static size_t MAX_PALETTES = 254;
const static size_t MAX_FRAME_OBJECTS = 32;
const static size_t MAX_ACTION_POINTS = 8;
const static size_t MAX_ENTITY_HITBOXES = 4;
const static size_t MAX_ANIMATION_FRAMES = 126;

const static size_t PALETTE_COLORS = 16;

typedef UnsignedBits<3, uint_fast8_t> SpriteOrderType;
const static SpriteOrderType DEFAULT_SPRITE_ORDER = 2;

enum class ObjectSize {
    SMALL = 8,
    LARGE = 16
};

typedef ClampedType<uint8_t, 1, 255> ActionPointParameter;

struct NameReference {
    idstring name;
    bool hFlip;
    bool vFlip;

    NameReference() = default;

    bool operator==(const NameReference& o) const
    {
        return name == o.name && hFlip == o.hFlip && vFlip == o.vFlip;
    }

    bool operator!=(const NameReference& o) const
    {
        return name != o.name || hFlip != o.hFlip || vFlip != o.vFlip;
    }
};
}
}
