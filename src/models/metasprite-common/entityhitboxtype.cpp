#include "entityhitboxtype.h"

using namespace UnTech::MetaSpriteCommon;

const std::map<EntityHitboxType::Enum, std::string> EntityHitboxType::enumMap = {
    { EntityHitboxType::Enum::BODY, "BODY" },
    { EntityHitboxType::Enum::BODY_WEAK, "BODY_WEAK" },
    { EntityHitboxType::Enum::SHIELD, "SHIELD" },
    { EntityHitboxType::Enum::SHIELD_ATTACK, "SHIELD_ATTACK" },
    { EntityHitboxType::Enum::ATTACK, "ATTACK" },
    { EntityHitboxType::Enum::PROJECTILE, "PROJECTILE" },
};

const std::map<std::string, EntityHitboxType::Enum> EntityHitboxType::stringMap = {
    { "BODY", EntityHitboxType::Enum::BODY },
    { "BODY_WEAK", EntityHitboxType::Enum::BODY_WEAK },
    { "SHIELD", EntityHitboxType::Enum::SHIELD },
    { "SHIELD_ATTACK", EntityHitboxType::Enum::SHIELD_ATTACK },
    { "ATTACK", EntityHitboxType::Enum::ATTACK },
    { "PROJECTILE", EntityHitboxType::Enum::PROJECTILE },
};
