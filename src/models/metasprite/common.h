/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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

namespace UnTech {
namespace MetaSprite {

const static size_t MAX_FRAMESETS = 1024;
const static size_t MAX_EXPORT_NAMES = 254;
const static size_t MAX_PALETTES = 254;
const static size_t MAX_FRAME_OBJECTS = 32;
const static size_t MAX_ACTION_POINTS = 8;
const static size_t MAX_ENTITY_HITBOXES = 4;
const static size_t MAX_ANIMATION_FRAMES = 126;

const static size_t MAX_ACTION_POINT_FUNCTIONS = 126;

const static size_t PALETTE_COLORS = 16;

typedef UnsignedBits<3, uint_fast8_t> SpriteOrderType;
const static SpriteOrderType DEFAULT_SPRITE_ORDER = 2;

enum class ObjectSize {
    SMALL = 8,
    LARGE = 16
};

struct NameReference {
    idstring name;
    bool hFlip;
    bool vFlip;

    NameReference() = default;

    std::string str() const;

    bool operator==(const NameReference& o) const
    {
        return name == o.name && hFlip == o.hFlip && vFlip == o.vFlip;
    }

    bool operator!=(const NameReference& o) const
    {
        return name != o.name || hFlip != o.hFlip || vFlip != o.vFlip;
    }
};

struct ActionPointFunction {
    idstring name;

    // If true then the project-compiler will create a define & constant in the
    // "Project.ActionPoints" namespace.
    bool manuallyInvoked = false;

    bool operator==(const ActionPointFunction& o) const
    {
        return name == o.name
               && manuallyInvoked == o.manuallyInvoked;
    }
};

using ActionPointMapping = std::unordered_map<idstring, uint8_t>;

std::shared_ptr<const ActionPointMapping>
generateActionPointMapping(const NamedList<ActionPointFunction>& apFunctions, ErrorList& err);

}
}
