/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityhitboxtype.h"

using namespace UnTech::MetaSprite;

const std::map<EntityHitboxType::Enum, std::string> EntityHitboxType::enumMap = {
    { EntityHitboxType::Enum::BODY, "BODY" },
    { EntityHitboxType::Enum::BODY_WEAK, "BODY_WEAK" },
    { EntityHitboxType::Enum::BODY_ATTACK, "BODY_ATTACK" },
    { EntityHitboxType::Enum::SHIELD, "SHIELD" },
    { EntityHitboxType::Enum::SHIELD_ATTACK, "SHIELD_ATTACK" },
    { EntityHitboxType::Enum::ATTACK, "ATTACK" },
};

const std::map<std::string, EntityHitboxType::Enum> EntityHitboxType::stringMap = {
    { "BODY", EntityHitboxType::Enum::BODY },
    { "BODY_WEAK", EntityHitboxType::Enum::BODY_WEAK },
    { "BODY_ATTACK", EntityHitboxType::Enum::BODY_ATTACK },
    { "SHIELD", EntityHitboxType::Enum::SHIELD },
    { "SHIELD_ATTACK", EntityHitboxType::Enum::SHIELD_ATTACK },
    { "ATTACK", EntityHitboxType::Enum::ATTACK },
};
