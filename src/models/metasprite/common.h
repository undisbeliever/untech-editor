#pragma once
#include "models/common/clampedinteger.h"
#include "models/common/unsignedbits.h"
#include <cstdint>
#include <string>

namespace UnTech {
namespace MetaSprite {

const static size_t MAX_FRAMES = 254;
const static size_t MAX_PALETTES = 254;
const static size_t MAX_ANIMATIONS = 254;
const static size_t MAX_FRAME_OBJECTS = 32;
const static size_t MAX_ACTION_POINTS = 8;
const static size_t MAX_ENTITY_HITBOXES = 4;
const static size_t MAX_ANIMATION_INSTRUCTIONS = 64;

typedef UnsignedBits<3, uint_fast8_t> SpriteOrderType;
const static SpriteOrderType DEFAULT_SPRITE_ORDER = 2;

enum class ObjectSize {
    SMALL = 8,
    LARGE = 16
};

typedef ClampedInteger<uint8_t, 1, 255> ActionPointParameter;

struct FrameReference {
    // ::TODO replace with idname::
    std::string frameName;
    bool hFlip;
    bool vFlip;

    FrameReference() = default;
    FrameReference(const FrameReference&) = default;
    FrameReference(FrameReference&&) = default;

    bool operator==(const FrameReference& o)
    {
        return frameName == o.frameName && hFlip == o.hFlip && vFlip == o.vFlip;
    }

    bool operator!=(const FrameReference& o)
    {
        return frameName != o.frameName || hFlip != o.hFlip || vFlip != o.vFlip;
    }
};
}
}
