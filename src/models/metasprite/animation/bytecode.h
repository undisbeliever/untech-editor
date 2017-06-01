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

class Bytecode {
public:
    enum class Enum : uint_fast8_t {
        STOP = 0x00,
        GOTO_START = 0x02,
        GOTO_ANIMATION = 0x04,
        GOTO_OFFSET = 0x06,
        SET_FRAME_AND_WAIT_FRAMES = 0x08,
        SET_FRAME_AND_WAIT_TIME = 0x0A,
        SET_FRAME_AND_WAIT_XVECL = 0x0C,
        SET_FRAME_AND_WAIT_YVECL = 0x0E
    };

    static const std::map<Enum, std::string> enumMap;
    static const std::map<std::string, Enum> stringMap;

    Bytecode(const Enum v = Enum::STOP)
        : _value(v)
    {
    }

    Bytecode(const std::string str)
        : _value(stringMap.at(str))
    {
    }

    unsigned instructionSize() const;

    // Returns true if the instruction is a jump or a stop
    bool isValidTerminator() const;

    bool usesFrame() const;
    bool usesGotoLabel() const;
    bool usesParameter() const;
    bool parameterUnsigned() const;

    Enum value() const { return _value; }
    const std::string& string() const { return enumMap.at(_value); }
    uint8_t engineValue() const { return (uint8_t)_value; }

    inline operator Enum() const { return _value; }

    inline bool operator==(const Bytecode& o) const { return _value == o._value; }
    inline bool operator==(Enum e) const { return _value == e; }

    inline bool operator!=(const Bytecode& o) const { return _value != o._value; }
    inline bool operator!=(Enum e) const { return _value != e; }

private:
    Enum _value;
};
}
}
}
