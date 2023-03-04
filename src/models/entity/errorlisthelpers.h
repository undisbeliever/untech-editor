/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromdata-error.h"
#include "entityromdata.h"

namespace UnTech::Entity {

template <typename... Args>
inline std::unique_ptr<EntityError> listIdError(const unsigned index, const Args... message)
{
    return std::make_unique<EntityError>(EntityErrorType::LIST_ID, index,
                                         stringBuilder(message...));
}

template <typename... Args>
inline std::unique_ptr<EntityError> structFieldError(const EntityRomStruct& rs, const unsigned structIndex, const unsigned fieldIndex,
                                                     const Args... message)
{
    return std::make_unique<EntityError>(EntityErrorType::STRUCT_FIELD, fieldIndex, structIndex,
                                         stringBuilder(u8"EntityRomStruct ", rs.name, u8", Field #", fieldIndex, u8": ", message...));
}

template <typename... Args>
inline std::unique_ptr<EntityError> structFieldError(const EntityRomStruct& rs, const StructField& field, const unsigned structIndex, const unsigned fieldIndex,
                                                     const Args... message)
{
    if (field.name.isValid()) {
        return std::make_unique<EntityError>(EntityErrorType::STRUCT_FIELD, fieldIndex, structIndex,
                                             stringBuilder(u8"EntityRomStruct ", rs.name, u8", Field ", field.name, u8": ", message...));
    }
    else {
        return structFieldError(rs, structIndex, fieldIndex, message...);
    }
}

template <typename... Args>
inline std::unique_ptr<EntityError> entityRomStructError(const EntityRomStruct& rs, const unsigned structIndex,
                                                         const Args... message)
{
    return std::make_unique<EntityError>(EntityErrorType::STRUCT, structIndex,
                                         stringBuilder(u8"EntityRomStruct ", rs.name, u8": ", message...));
}

template <typename... Args>
inline std::unique_ptr<EntityError> entityFunctionTableError(const EntityFunctionTable& ft, const unsigned ftIndex,
                                                             const Args... message)
{
    return std::make_unique<EntityError>(EntityErrorType::ENTITY_FUNCTION_TABLE, ftIndex,
                                         stringBuilder(u8"EntityFunctionTable ", ft.name, u8": ", message...));
}

inline EntityErrorType entityRomEntryErrorType(const EntityType entityType)
{
    switch (entityType) {
    case EntityType::ENTITY:
        return EntityErrorType::ENTITY_ROM_ENTRY;

    case EntityType::PROJECTILE:
        return EntityErrorType::PROJECTILE_ROM_ENTRY;

    case EntityType::PLAYER:
        return EntityErrorType::PLAYER_ROM_ENTRY;
    }

    return EntityErrorType::ENTITY_ROM_ENTRY;
}

template <typename... Args>
inline std::unique_ptr<EntityError> entityRomEntryError(const EntityRomEntry& re, const EntityType entityType, const unsigned index,
                                                        const Args... message)
{
    return std::make_unique<EntityError>(entityRomEntryErrorType(entityType), index,
                                         stringBuilder(u8"EntityRomEntry ", re.name, u8": ", message...));
}

}
