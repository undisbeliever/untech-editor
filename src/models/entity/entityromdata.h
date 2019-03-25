/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
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

namespace UnTech {
namespace Project {
struct ProjectFile;
}

namespace Entity {

enum class DataType {
    UINT8,
    UINT16,
    UINT24,
    UINT32,
    SINT8,
    SINT16,
    SINT24,
    SINT32,
    ADDR,
};

enum class ParameterType {
    UNUSED,
    WORD,
    SIGNED_WORD,
};

struct StructField {
    idstring name;
    DataType type;
    std::string defaultValue; // may be empty
    std::string comment;

    StructField() = default;

    bool validate(ErrorList& err) const;

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

    // NOTE: Does not validate parent
    bool validate(ErrorList& err) const;

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
    idstring entityStruct;
    idstring exportOrder;
    ParameterType parameterType;
    std::string comment;

    EntityFunctionTable() = default;

    // Does not validate name unique or entityStruct exists
    bool validate(ErrorList& err) const;

    bool operator==(const EntityFunctionTable& o) const
    {
        return name == o.name
               && entityStruct == o.entityStruct
               && exportOrder == o.exportOrder
               && parameterType == o.parameterType
               && comment == o.comment;
    }
};
using FunctionTableMap = std::unordered_map<idstring,
                                            std::pair<const EntityFunctionTable*, const FieldList*>>;

// Also validates the EntityRomDataStruct entries
StructFieldMap generateStructMap(const NamedList<EntityRomStruct>& structs, ErrorList& err);

// Also validates functionTables
// WARNING: only valid for the lifetime of StructFieldMap
FunctionTableMap generateFunctionTableFieldMap(const NamedList<EntityFunctionTable>& functionTables,
                                               const StructFieldMap& structFieldMap,
                                               ErrorList& err);

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

    bool validate(const Project::ProjectFile& project, const FunctionTableMap& ftMap, ErrorList& err) const;

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

struct EntityRomData {
    std::vector<idstring> listIds;

    NamedList<EntityRomStruct> structs;
    NamedList<EntityFunctionTable> functionTables;

    NamedList<EntityRomEntry> entities;
    NamedList<EntityRomEntry> projectiles;

    bool validateListIds(ErrorList& err) const;

    bool operator==(const EntityRomData& o) const
    {
        return listIds == o.listIds
               && structs == o.structs
               && functionTables == o.functionTables
               && entities == o.entities
               && projectiles == projectiles;
    }
};

struct CompiledEntityRomData {
    const static int ENTITY_FORMAT_VERSION;
    const static std::string ENTITY_INDEXES_LABEL;
    const static std::string PROJECTILE_INDEXES_LABEL;

    std::vector<uint8_t> entityIndexes;
    std::vector<uint8_t> projectileIndexes;

    std::string defines;
    std::string entries;

    bool valid;
};
CompiledEntityRomData compileEntityRomData(const EntityRomData& data, const Project::ProjectFile& project, ErrorList& err);

}
}
