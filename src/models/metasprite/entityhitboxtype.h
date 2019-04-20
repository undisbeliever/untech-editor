/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/enummap.h"
#include <array>
#include <cstdint>

namespace UnTech {
namespace MetaSprite {

struct EntityHitboxType {
    bool weak = 0;
    bool shield = 0;
    bool attack = 0;
    bool body = 0;

    static const std::array<std::string, 16> SHORT_STRING_VALUES;
    static const std::array<std::string, 16> LONG_STRING_VALUES;

    uint8_t romValue() const
    {
        return (weak << 3) | (shield << 2) | (attack << 1) | (body << 0);
    }

    static EntityHitboxType from_romValue(uint8_t v)
    {
        EntityHitboxType eht;

        eht.weak = v & (1 << 3);
        eht.shield = v & (1 << 2);
        eht.attack = v & (1 << 1);
        eht.body = v & (1 << 0);

        return eht;
    }

    const std::string& to_string() const
    {
        return SHORT_STRING_VALUES.at(romValue() & 0xf);
    }

    const std::string& to_long_string() const
    {
        return LONG_STRING_VALUES.at(romValue() & 0xf);
    }

    static EntityHitboxType from_string(const std::string& string);

    inline bool operator==(const EntityHitboxType& o) const
    {
        return weak == o.weak
               && shield == o.shield
               && attack == o.attack
               && body == o.body;
    }
    inline bool operator!=(const EntityHitboxType& o) const { return !(*this == o); }
};
}
}
