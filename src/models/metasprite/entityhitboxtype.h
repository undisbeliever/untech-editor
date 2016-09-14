#pragma once

#include <cstdint>
#include <map>
#include <string>

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
    EntityHitboxType(EntityHitboxType&&) = default;

    static EntityHitboxType smallestFixedTileset(unsigned tilesetSize);

    Enum value() const { return _value; };
    uint8_t romValue() const { return (uint8_t)_value; }

    const std::string& string() const { return enumMap.at(_value); }

    inline operator Enum() const { return _value; }
    inline EntityHitboxType operator=(const EntityHitboxType& v)
    {
        _value = v._value;
        return *this;
    }

    inline bool operator==(const EntityHitboxType& o) const { return _value == o._value; }
    inline bool operator==(Enum e) const { return _value == e; }

    inline bool operator!=(const EntityHitboxType& o) const { return _value != o._value; }
    inline bool operator!=(Enum e) const { return _value != e; }

private:
    Enum _value;
};
}
}
