/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/enummap.h"
#include <cstdint>

namespace UnTech {
namespace MetaSprite {

class EntityHitboxType {
public:
    enum class Enum : uint_fast8_t {
        BODY = 0,
        BODY_WEAK = 2,
        BODY_ATTACK = 4,
        SHIELD = 6,
        SHIELD_ATTACK = 8,
        ATTACK = 10,
    };

    static const EnumMap<Enum> enumMap;

    EntityHitboxType(const Enum v = Enum::BODY)
        : _value(v)
    {
    }

    EntityHitboxType(const std::string str)
        : _value(enumMap.valueOf(str))
    {
    }

    static EntityHitboxType smallestFixedTileset(unsigned tilesetSize);

    Enum value() const { return _value; }
    uint8_t romValue() const { return (uint8_t)_value; }

    const std::string& string() const { return enumMap.nameOf(_value); }

    inline operator Enum() const { return _value; }

    inline bool operator==(const EntityHitboxType& o) const { return _value == o._value; }
    inline bool operator==(Enum e) const { return _value == e; }

    inline bool operator!=(const EntityHitboxType& o) const { return _value != o._value; }
    inline bool operator!=(Enum e) const { return _value != e; }

private:
    Enum _value;
};
}
}
