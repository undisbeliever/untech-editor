#pragma once

#include <cstdint>
#include <map>
#include <string>

namespace UnTech {
namespace MetaSpriteCommon {

class EntityHitboxType {
public:
    enum class Enum : uint_fast8_t {
        BODY = 0,
        BODY_WEAK = 2,
        SHIELD = 4,
        SHIELD_ATTACK = 6,
        ATTACK = 8,
        PROJECTILE = 10,
    };

    static const std::map<Enum, std::string> enumMap;
    static const std::map<std::string, Enum> stringMap;

    EntityHitboxType(const Enum v = Enum::BODY)
        : _value(v)
    {
    }

    EntityHitboxType(const std::string str)
        : _value(stringMap.at(str))
    {
    }

    EntityHitboxType(const EntityHitboxType&) = default;

    static EntityHitboxType smallestFixedTileset(unsigned tilesetSize);

    Enum value() const { return _value; };
    uint8_t romValue() const { return (uint8_t)_value; }

    const std::string& string() const { return enumMap.at(_value); }

    bool operator==(const EntityHitboxType& o) const
    {
        return _value == o._value;
    }
    bool operator==(Enum e) const
    {
        return _value == e;
    }

    bool operator!=(const EntityHitboxType& o) const
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
