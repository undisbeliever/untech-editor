/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromdata-error.h"

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
                                         stringBuilder("EntityRomStruct ", rs.name, ", Field #", fieldIndex, ": ", message...));
}

template <typename... Args>
inline std::unique_ptr<EntityError> structFieldError(const EntityRomStruct& rs, const StructField& field, const unsigned structIndex, const unsigned fieldIndex,
                                                     const Args... message)
{
    if (field.name.isValid()) {
        return std::make_unique<EntityError>(EntityErrorType::STRUCT_FIELD, fieldIndex, structIndex,
                                             stringBuilder("EntityRomStruct ", rs.name, ", Field ", field.name, ": ", message...));
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
                                         stringBuilder("EntityRomStruct ", rs.name, ": ", message...));
}

template <typename... Args>
inline std::unique_ptr<EntityError> entityFunctionTableError(const EntityFunctionTable& ft, const unsigned ftIndex,
                                                             const Args... message)
{
    return std::make_unique<EntityError>(EntityErrorType::ENTITY_FUNCTION_TABLE, ftIndex,
                                         stringBuilder("EntityFunctionTable ", ft.name, ": ", message...));
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
                                         stringBuilder("EntityRomEntry ", re.name, ": ", message...));
}

}
