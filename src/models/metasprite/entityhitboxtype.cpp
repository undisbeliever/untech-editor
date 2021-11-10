/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityhitboxtype.h"

namespace UnTech::MetaSprite {

const std::array<std::u8string, 16> EntityHitboxType::SHORT_STRING_VALUES({
    u8"----",
    u8"---B",
    u8"--A-",
    u8"--AB",
    u8"-S--",
    u8"-S-B",
    u8"-SA-",
    u8"-SAB",
    u8"W---",
    u8"W--B",
    u8"W-A-",
    u8"W-AB",
    u8"WS--",
    u8"WS-B",
    u8"WSA-",
    u8"WSAB",
});

const std::array<std::u8string, 16> EntityHitboxType::LONG_STRING_VALUES({
    u8"",
    u8"Body",
    u8"Attack",
    u8"Attack Body",
    u8"Shield",
    u8"Shield Body",
    u8"Shield Attack",
    u8"Shield Attack Body",
    u8"Weak",
    u8"Weak Body",
    u8"Weak Attack",
    u8"Weak Attack Body",
    u8"Weak Shield",
    u8"Weak Shield Body",
    u8"Weak Shield Attack",
    u8"Weak Shield Attack Body",
});

EntityHitboxType EntityHitboxType::from_string(const std::u8string& string)
{
    // Includes backwards compatability with old string values

    using EHT = EntityHitboxType;
    const std::array<std::pair<std::u8string, EHT>, 6> OLD_VALUES = { {
        { u8"BODY", []() { EHT t; t.body = true; return t; }() },
        { u8"BODY_WEAK", []() { EHT t; t.body = t.weak =true; return t; }() },
        { u8"BODY_ATTACK", []() { EHT t; t.body = t.attack = true; return t; }() },
        { u8"SHIELD", []() { EHT t; t.shield = true; return t; }() },
        { u8"SHIELD_ATTACK", []() { EHT t; t.shield = t.attack = true; return t; }() },
        { u8"ATTACK", []() { EHT t; t.attack = true; return t; }() },
    } };

    for (auto ov : OLD_VALUES) {
        if (ov.first == string) {
            return ov.second;
        }
    }

    if (string.size() > 4) {
        throw out_of_range(u8"Cannot convert `", string, u8"` to EntityHitboxType");
    }

    // Order does not matter (in the event I change the order)

    EntityHitboxType eht;

    for (const char8_t c : string) {
        if (c == '-') {
            continue;
        }
        else if (c == 'W' || c == 'w') {
            eht.weak = true;
        }
        else if (c == 'S' || c == 's') {
            eht.shield = true;
        }
        else if (c == 'A' || c == 'a') {
            eht.attack = true;
        }
        else if (c == 'B' || c == 'b') {
            eht.body = true;
        }
        else {
            throw out_of_range(u8"Cannot convert `", string, u8"` to EntityHitboxType");
        }
    }

    return eht;
}

}
