/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"

namespace UnTech::Entity {

enum class EntityErrorType {
    LIST_ID,
    STRUCT,
    STRUCT_FIELD,
    ENTITY_FUNCTION_TABLE,
    ENTITY_ROM_ENTRY,
    PROJECTILE_ROM_ENTRY,
    PLAYER_ROM_ENTRY,
};
using EntityError = GenericListError<EntityErrorType>;

}
