/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/errorlist.h"
#include "models/common/idstring.h"
#include "models/common/namedlist.h"
#include <memory>
#include <unordered_map>
#include <vector>

namespace UnTech::Project {
struct ProjectFile;
}

namespace UnTech::Entity {

constexpr unsigned MAX_N_ENTITY_ENTRIES = 256;

enum class EntityType {
    ENTITY,
    PROJECTILE,
    PLAYER,
};

enum class DataType {
    UINT8,
    UINT16,
    UINT24,
    UINT32,
    SINT8,
    SINT16,
    SINT24,
    SINT32,
};

enum class ParameterType {
    UNUSED,
    UNSIGNED_BYTE,
};

struct EntityRomStruct;

struct StructField {
    idstring name;
    DataType type = DataType::UINT8;
    std::u8string defaultValue; // may be empty
    std::u8string comment;

    bool operator==(const StructField&) const = default;
};

struct EntityRomStruct {
    idstring name;
    idstring parent; // may be empty
    std::u8string comment;

    std::vector<StructField> fields;

    EntityRomStruct() = default;

    bool operator==(const EntityRomStruct&) const = default;
};

using FieldList = std::vector<StructField>;
using StructFieldMap = std::unordered_map<idstring, FieldList>;

struct EntityFunctionTable {
    idstring name;
    EntityType entityType = EntityType::ENTITY;
    idstring entityStruct;
    idstring exportOrder;
    ParameterType parameterType = ParameterType::UNUSED;
    std::u8string comment;

    EntityFunctionTable() = default;

    bool operator==(const EntityFunctionTable&) const = default;
};

struct EntityRomEntry {
    idstring name;
    idstring functionTable;
    std::u8string comment;

    idstring initialProjectileId;
    idstring initialListId;

    idstring frameSetId;
    idstring displayFrame;

    unsigned defaultPalette = 0;

    std::unordered_map<idstring, std::u8string> fields;

    bool operator==(const EntityRomEntry&) const = default;
};

struct EntityRomData {
    std::vector<idstring> listIds;

    NamedList<EntityRomStruct> structs;
    NamedList<EntityFunctionTable> functionTables;

    NamedList<EntityRomEntry> entities;
    NamedList<EntityRomEntry> projectiles;
    NamedList<EntityRomEntry> players;

    bool operator==(const EntityRomData&) const = default;
};

struct CompiledEntityRomData {
    const static int ENTITY_FORMAT_VERSION;

    const static std::u8string ROM_DATA_LABEL;
    const static std::u8string ROM_DATA_LIST_LABEL;

    std::u8string defines;
    std::u8string functionTableData;

    std::vector<uint8_t> romDataIndexes;
    std::vector<uint8_t> romData;

    std::unordered_map<idstring, std::pair<unsigned, ParameterType>> entityNameMap;

    bool valid;
};

std::shared_ptr<const CompiledEntityRomData>
compileEntityRomData(const EntityRomData& data, const Project::ProjectFile& project, ErrorList& err);

}
