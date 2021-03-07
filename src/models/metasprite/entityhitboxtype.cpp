/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityhitboxtype.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

const std::array<std::string, 16> EntityHitboxType::SHORT_STRING_VALUES({
    "----",
    "---B",
    "--A-",
    "--AB",
    "-S--",
    "-S-B",
    "-SA-",
    "-SAB",
    "W---",
    "W--B",
    "W-A-",
    "W-AB",
    "WS--",
    "WS-B",
    "WSA-",
    "WSAB",
});

const std::array<std::string, 16> EntityHitboxType::LONG_STRING_VALUES({
    "",
    "Body",
    "Attack",
    "Attack Body",
    "Shield",
    "Shield Body",
    "Shield Attack",
    "Shield Attack Body",
    "Weak",
    "Weak Body",
    "Weak Attack",
    "Weak Attack Body",
    "Weak Shield",
    "Weak Shield Body",
    "Weak Shield Attack",
    "Weak Shield Attack Body",
});

EntityHitboxType EntityHitboxType::from_string(const std::string& string)
{
    // Includes backwards compatability with old string values

    using EHT = EntityHitboxType;
    const std::array<std::pair<std::string, EHT>, 6> OLD_VALUES = { {
        { "BODY", []() { EHT t; t.body = true; return t; }() },
        { "BODY_WEAK", []() { EHT t; t.body = t.weak =true; return t; }() },
        { "BODY_ATTACK", []() { EHT t; t.body = t.attack = true; return t; }() },
        { "SHIELD", []() { EHT t; t.shield = true; return t; }() },
        { "SHIELD_ATTACK", []() { EHT t; t.shield = t.attack = true; return t; }() },
        { "ATTACK", []() { EHT t; t.attack = true; return t; }() },
    } };

    for (auto ov : OLD_VALUES) {
        if (ov.first == string) {
            return ov.second;
        }
    }

    if (string.size() > 4) {
        throw std::out_of_range(stringBuilder("Cannot convert `", string, "` to EntityHitboxType"));
    }

    // Order does not matter (in the event I change the order)

    EntityHitboxType eht;

    for (const char& c : string) {
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
            throw std::out_of_range(stringBuilder("Cannot convert `", string, "` to EntityHitboxType"));
        }
    }

    return eht;
}
