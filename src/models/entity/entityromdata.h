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
#include <ostream>
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
    DataType type;
    std::string defaultValue; // may be empty
    std::string comment;

    StructField() = default;

    bool operator==(const StructField& o) const
    {
        return name == o.name
               && type == o.type
               && defaultValue == o.defaultValue
               && comment == o.comment;
    }
};

struct EntityRomStruct {
    idstring name;
    idstring parent; // may be empty
    std::string comment;

    std::vector<StructField> fields;

    EntityRomStruct() = default;

    bool operator==(const EntityRomStruct& o) const
    {
        return name == o.name
               && parent == o.parent
               && comment == o.comment
               && fields == o.fields;
    }
};

using FieldList = std::vector<StructField>;
using StructFieldMap = std::unordered_map<idstring, FieldList>;

struct EntityFunctionTable {
    idstring name;
    EntityType entityType;
    idstring entityStruct;
    idstring exportOrder;
    ParameterType parameterType;
    std::string comment;

    EntityFunctionTable() = default;

    bool operator==(const EntityFunctionTable& o) const
    {
        return name == o.name
               && entityType == o.entityType
               && entityStruct == o.entityStruct
               && exportOrder == o.exportOrder
               && parameterType == o.parameterType
               && comment == o.comment;
    }
};

using FunctionTableMap = std::unordered_map<idstring,
                                            std::pair<const EntityFunctionTable*, const FieldList*>>;

struct EntityRomEntry {
    idstring name;
    idstring functionTable;
    std::string comment;

    idstring initialProjectileId;
    idstring initialListId;

    idstring frameSetId;
    idstring displayFrame;

    unsigned defaultPalette = 0;

    std::unordered_map<idstring, std::string> fields;

    bool operator==(const EntityRomEntry& o) const
    {
        return name == o.name
               && functionTable == o.functionTable
               && comment == o.comment
               && initialProjectileId == o.initialProjectileId
               && initialListId == o.initialListId
               && frameSetId == o.frameSetId
               && displayFrame == o.displayFrame
               && fields == o.fields;
    }
};

// Also validates the EntityRomDataStruct entries
StructFieldMap generateStructMap(const NamedList<EntityRomStruct>& structs, ErrorList& err);

struct EntityRomData {
    std::vector<idstring> listIds;

    NamedList<EntityRomStruct> structs;
    NamedList<EntityFunctionTable> functionTables;

    NamedList<EntityRomEntry> entities;
    NamedList<EntityRomEntry> projectiles;
    NamedList<EntityRomEntry> players;

    bool operator==(const EntityRomData& o) const
    {
        return listIds == o.listIds
               && structs == o.structs
               && functionTables == o.functionTables
               && entities == o.entities
               && projectiles == o.projectiles
               && players == o.players;
    }
};

struct CompiledEntityRomData {
    const static int ENTITY_FORMAT_VERSION;

    const static std::string ROM_DATA_LABEL;
    const static std::string ROM_DATA_LIST_LABEL;

    std::string defines;
    std::string functionTableData;

    std::vector<uint8_t> romDataIndexes;
    std::vector<uint8_t> romData;

    std::unordered_map<idstring, std::pair<unsigned, ParameterType>> entityNameMap;

    bool valid;
};

std::shared_ptr<const CompiledEntityRomData>
compileEntityRomData(const EntityRomData& data, const Project::ProjectFile& project, ErrorList& err);

}
