/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace UnTech {
namespace MetaSprite {
namespace Animation {

class DurationFormat {
public:
    enum class Enum : uint_fast8_t {
        FRAME = 0x00,
        TIME = 0x02,
        DISTANCE_VERTICAL = 0x04,
        DISTANCE_HORIZONTAL = 0x06,
    };

    static const std::map<Enum, std::string> enumMap;
    static const std::map<std::string, Enum> stringMap;

    DurationFormat(const Enum v = Enum::FRAME)
        : _value(v)
    {
    }

    DurationFormat(const std::string str)
        : _value(stringMap.at(str))
    {
    }

    // Converts the duration integer into a united string
    std::string durationToString(uint8_t duration) const;

    Enum value() const { return _value; }
    const std::string& string() const { return enumMap.at(_value); }
    uint8_t engineValue() const { return (uint8_t)_value; }

    inline operator Enum() const { return _value; }

    inline bool operator==(const DurationFormat& o) const { return _value == o._value; }
    inline bool operator==(Enum e) const { return _value == e; }

    inline bool operator!=(const DurationFormat& o) const { return _value != o._value; }
    inline bool operator!=(Enum e) const { return _value != e; }

private:
    Enum _value;
};
}
}
}