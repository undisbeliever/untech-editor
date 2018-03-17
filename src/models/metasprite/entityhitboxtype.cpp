/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityhitboxtype.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

const EnumMap<EntityHitboxType::Enum> EntityHitboxType::enumMap = {
    { "BODY", EntityHitboxType::Enum::BODY },
    { "BODY_WEAK", EntityHitboxType::Enum::BODY_WEAK },
    { "BODY_ATTACK", EntityHitboxType::Enum::BODY_ATTACK },
    { "SHIELD", EntityHitboxType::Enum::SHIELD },
    { "SHIELD_ATTACK", EntityHitboxType::Enum::SHIELD_ATTACK },
    { "ATTACK", EntityHitboxType::Enum::ATTACK },
};
