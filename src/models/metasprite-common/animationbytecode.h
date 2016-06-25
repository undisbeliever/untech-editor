#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace UnTech {
namespace MetaSpriteCommon {

class AnimationBytecode {
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

    AnimationBytecode(const Enum v = Enum::STOP)
        : _value(v)
    {
    }

    AnimationBytecode(const std::string str)
        : _value(stringMap.at(str))
    {
    }

    AnimationBytecode(const AnimationBytecode&) = default;

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

    bool operator==(const AnimationBytecode& o) const
    {
        return _value == o._value;
    }
    bool operator==(Enum e) const
    {
        return _value == e;
    }

    bool operator!=(const AnimationBytecode& o) const
    {
        return _value != o._value;
    }
    bool operator!=(Enum e) const
    {
        return _value != e;
    }

private:
    Enum _value;
};
}
}
