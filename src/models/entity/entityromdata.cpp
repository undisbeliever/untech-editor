/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromdata.h"
#include "entityromdata-error.h"
#include "errorlisthelpers.h"
#include "models/common/iterators.h"
#include "models/common/string.h"
#include "models/common/stringstream.h"
#include "models/project/project.h"
#include <algorithm>

#include <unordered_set>

namespace UnTech::Entity {

const int CompiledEntityRomData::ENTITY_FORMAT_VERSION = 6;

#define BASE_ROM_STRUCT u8"BaseEntityRomStruct"
#define ENTITY_ROM_STRUCT_NAMESPACE u8"Project.EntityRomStructs"
static const idstring baseRomStruct = u8"BASE_ROM_STRUCT"_id;

const std::unordered_set<idstring> INVALID_NAMES{
    u8"functionTable"_id,
    u8"defaultPalette"_id,
    u8"initialProjectileId"_id,
    u8"initialListId"_id,
    u8"frameSetId"_id,
    u8"Players"_id,
    u8"Projectiles"_id,
    u8"size"_id,
    u8"count"_id,
    u8"__STRUCT__"_id,
    baseRomStruct,
};

// WARNING: contains references
// Values are only valid (and safe) so long as `functionTables` and `structFieldMap remain unchanged
using FunctionTableMap = std::unordered_map<idstring,
                                            std::pair<const EntityFunctionTable&, const FieldList&>>;

static const char8_t* entityTypeString(const EntityType entityType)
{
    switch (entityType) {
    case EntityType::ENTITY:
        return u8"Entity";

    case EntityType::PROJECTILE:
        return u8"Projectile";

    case EntityType::PLAYER:
        return u8"Player";
    }

    return u8"";
}

static unsigned fieldSize(DataType type)
{
    switch (type) {
    case DataType::UINT8:
    case DataType::SINT8:
        return 1;

    case DataType::UINT16:
    case DataType::SINT16:
        return 2;

    case DataType::UINT24:
    case DataType::SINT24:
        return 3;

    case DataType::UINT32:
    case DataType::SINT32:
        return 4;
    }

    fputs("Invalid DataType\n", stderr);
    abort();
}

static const char8_t* fieldComment(DataType type)
{
    switch (type) {
    case DataType::UINT8:
        return u8"uint8";

    case DataType::UINT16:
        return u8"uint16";

    case DataType::UINT24:
        return u8"uint24";

    case DataType::UINT32:
        return u8"uint32";

    case DataType::SINT8:
        return u8"sint8";

    case DataType::SINT16:
        return u8"sint16";

    case DataType::SINT24:
        return u8"sint24";

    case DataType::SINT32:
        return u8"sint32";
    }

    fputs("Invalid DataType\n", stderr);
    abort();
}

static bool validateFieldValue(DataType type, const std::u8string& str)
{
    auto testUnsigned = [&](uint32_t max) -> bool {
        const auto v = String::decimalOrHexToUint32(str);
        return v && *v <= max;
    };
    auto testSigned = [&](int32_t min, int32_t max) -> bool {
        const auto v = String::decimalOrHexToInt32(str);
        return v && *v >= min && *v <= max;
    };

    switch (type) {
    case DataType::UINT8:
        return testUnsigned(UINT8_MAX);

    case DataType::UINT16:
        return testUnsigned(UINT16_MAX);

    case DataType::UINT24:
        return testUnsigned((1 << 24) - 1);

    case DataType::UINT32:
        return testUnsigned(UINT32_MAX);

    case DataType::SINT8:
        return testSigned(INT8_MIN, INT8_MAX);

    case DataType::SINT16:
        return testSigned(INT16_MIN, INT16_MAX);

    case DataType::SINT24:
        return testSigned(-(1 << 23), (1 << 23) - 1);

    case DataType::SINT32:
        return testSigned(INT32_MIN, INT32_MAX);
    }

    return false;
}

static bool validate(const StructField& input, const EntityRomStruct& romStruct, const unsigned structIndex, const unsigned fieldIndex,
                     ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addError(structFieldError(romStruct, structIndex, fieldIndex, msg...));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Missing field name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError(u8"Invalid field name ", input.name);
    }
    if (input.comment.find('\n') != std::u8string::npos) {
        addError(u8"Comment must not contain a new line");
    }

    if (input.defaultValue.empty() == false) {
        if (!validateFieldValue(input.type, input.defaultValue)) {
            addError(u8"Invalid defaultValue");
        }
    }

    return valid;
}

// NOTE: Does not validate parent
static bool validate(const EntityRomStruct& input, const unsigned structIndex, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addError(entityRomStructError(input, structIndex, msg...));
        valid = false;
    };
    auto addStructFieldError = [&](unsigned index, const auto... msg) {
        err.addError(structFieldError(input, structIndex, index, msg...));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Expected name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError(u8"Invalid name ", input.name);
    }
    if (input.name == baseRomStruct) {
        addError(u8"Name cannot be " BASE_ROM_STRUCT);
    }
    if (input.parent.isValid() && input.parent == input.name) {
        addError(u8"Parent cannot refer to self");
    }
    if (input.fields.empty()) {
        addError(u8"Expected at least one field");
    }
    if (input.comment.find('\n') != std::u8string::npos) {
        addError(u8"Comment must not contain a new line");
    }

    for (auto it = input.fields.cbegin(); it != input.fields.cend(); it++) {
        auto& field = *it;
        unsigned index = std::distance(input.fields.cbegin(), it);

        bool fieldValid = validate(field, input, structIndex, index, err);
        if (fieldValid) {
            auto dupIt = std::find_if(input.fields.begin(), it, [&](const auto& f) { return f.name == field.name; });
            if (dupIt != it) {
                addStructFieldError(index, u8"Duplicate field name detected: ", field.name);
            }
        }
    }

    return valid;
}

// Does not validate name unique or entityStruct exists
static bool validate(const EntityFunctionTable& input, const unsigned ftIndex, const Project::ProjectFile& project, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addError(entityFunctionTableError(input, ftIndex, msg...));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Missing EntityFunctionTable name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError(u8"Invalid EntityFunctionTable name ", input.name);
    }
    if (input.exportOrder.isValid() == false) {
        addError(u8"Missing export order");
    }
    if (input.comment.find('\n') != std::u8string::npos) {
        addError(u8"Comment must not contain a new line");
    }

    if (input.exportOrder.isValid()) {
        if (not project.frameSetExportOrders.find(input.exportOrder)) {
            addError(u8"Cannot find FrameSet Export Order ", input.exportOrder);
        }
    }

    return valid;
}

static bool validate(const EntityRomEntry& input, const EntityType entityType, const unsigned index,
                     const Project::ProjectFile& project, const FunctionTableMap& ftMap, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addError(entityRomEntryError(input, entityType, index, msg...));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError(u8"Expected name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError(u8"Invalid name ", input.name);
    }
    if (input.functionTable.isValid() == false) {
        addError(u8"Missing functionTable");
    }
    if (entityType != EntityType::PLAYER) {
        if (input.initialListId.isValid() == false) {
            addError(u8"Missing initialListId");
        }
    }
    if (input.frameSetId.isValid() == false) {
        addError(u8"Missing frameSetId");
    }
    if (input.displayFrame.isValid() == false) {
        addError(u8"Missing displayFrame");
    }
    if (input.comment.find('\n') != std::u8string::npos) {
        addError(u8"Comment must not contain a new line");
    }

    if (input.initialProjectileId.isValid()) {
        const auto& projectiles = project.entityRomData.projectiles;
        if (not projectiles.find(input.initialProjectileId)) {
            addError(u8"Unable to find projectile ", input.initialProjectileId);
        }
    }

    if (input.initialListId.isValid()) {
        auto& listIds = project.entityRomData.listIds;
        bool listIdValid = std::find(listIds.begin(), listIds.end(), input.initialListId) != listIds.end();
        if (!listIdValid) {
            addError(u8"Unable to find listId ", input.initialListId);
        }
    }

    auto fieldsIt = ftMap.find(input.functionTable);
    if (fieldsIt != ftMap.end()) {
        const EntityFunctionTable& ft = fieldsIt->second.first;
        const FieldList& ftFields = fieldsIt->second.second;

        if (ft.entityType != entityType) {
            addError(u8"Function Table is the wrong type (expected ", entityTypeString(entityType), u8" but ", ft.name, u8" is a ", entityTypeString(ft.entityType), u8")");
        }

        for (auto& field : ftFields) {
            auto fIt = input.fields.find(field.name);
            if (fIt != input.fields.end()) {
                const auto& value = fIt->second;
                if (value.empty()) {
                    if (field.defaultValue.empty()) {
                        addError(u8"Missing field ", field.name);
                    }
                }
                else {
                    if (validateFieldValue(field.type, value) == false) {
                        addError(u8"Invalid field ", field.name, u8": ", value);
                    }
                }
            }
        }
    }
    else {
        addError(u8"Unable to retrieve field list for functionTable ", input.functionTable);
    }

    if (input.frameSetId.isValid()) {
        auto frameSetIt = std::find_if(project.frameSets.begin(), project.frameSets.end(),
                                       [&](auto& fs) { return fs.name() == input.frameSetId; });
        if (frameSetIt != project.frameSets.end()) {
            auto testFrameSet = [&](const auto& frameSet, unsigned nPalettes) {
                if (nPalettes == 0) {
                    nPalettes = 1;
                }

                if (fieldsIt != ftMap.end()) {
                    const EntityFunctionTable& fTable = fieldsIt->second.first;
                    if (fTable.exportOrder != frameSet.exportOrder) {
                        addError(u8"export order for frameSet ", frameSet.name, u8" is not ", fTable.exportOrder);
                    }
                }
                if (input.displayFrame.isValid() && !frameSet.frames.find(input.displayFrame)) {
                    addError(u8"Unable to find frame ", input.displayFrame);
                }
                if (input.defaultPalette >= nPalettes) {
                    addError(u8"Invalid defaultPalette (value must be < ", nPalettes, u8")");
                }
            };

            auto& fs = *frameSetIt;
            if (fs.siFrameSet) {
                testFrameSet(*fs.siFrameSet, fs.siFrameSet->palette.nPalettes);
            }
            else if (fs.msFrameSet) {
                testFrameSet(*fs.msFrameSet, fs.msFrameSet->palettes.size());
            }
            else {
                addError(u8"Unable to read frameSet ", input.frameSetId);
            }
        }
        else {
            addError(u8"Unable to find frameSet ", input.frameSetId);
        }
    }

    return valid;
}

bool validateListIds(const std::vector<idstring>& listIds, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto&... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    auto addListIdError = [&](unsigned index, const auto&... msg) {
        err.addError(listIdError(index, msg...));
        valid = false;
    };

    if (listIds.empty()) {
        addError(u8"Expected at least one Entity List Id");
    }

    for (auto [index, listId] : const_enumerate(listIds)) {
        if (listId.isValid() == false) {
            addListIdError(index, u8"Missing listId name");
            continue;
        }
        if (INVALID_NAMES.find(listId) != INVALID_NAMES.end()) {
            addListIdError(index, u8"Invalid ListId name ", listId);
            continue;
        }

        auto count = std::count(listIds.cbegin(), listIds.cbegin() + index, listId);
        if (count == 1) {
            addListIdError(index, u8"Duplicate listId detected: ", listId);
        }
    }

    return valid;
}

StructFieldMap generateStructMap(const NamedList<EntityRomStruct>& structs, ErrorList& err)
{
    std::vector<unsigned> toProcess(structs.size());
    {
        auto it = toProcess.begin();
        for (auto [i, s] : const_enumerate(structs)) {
            validate(s, i, err);

            *it++ = i;
        }
        assert(it == toProcess.end());
    }

    StructFieldMap fieldMap(toProcess.size() + 1);
    fieldMap.emplace(idstring(), FieldList());

    // returns true if the struct is to be removed from the process list
    auto processStruct = [&](const unsigned structIndex) -> bool {
        const EntityRomStruct& s = structs.at(structIndex);

        // Check name is unique and valid
        if (!s.name.isValid()) {
            return true;
        }
        auto it = fieldMap.find(s.name);
        if (it != fieldMap.end()) {
            err.addError(entityRomStructError(s, structIndex, u8"Duplicate name detected"));
            return true;
        }

        if (s.parent.isValid() == false) {
            // no parent

            fieldMap.emplace(s.name, s.fields);
            return true;
        }
        else {
            auto parentIt = fieldMap.find(s.parent);
            if (parentIt == fieldMap.end()) {
                // parent hasn't been processed yet
                return false;
            }

            // struct has a parent and the parent has already been processed

            const auto& parentFields = parentIt->second;

            for (auto f : const_enumerate(s.fields)) {
                bool overridesField = std::any_of(parentFields.begin(), parentFields.end(), [&](const auto& o) { return f.second.name == o.name; });
                if (overridesField) {
                    err.addError(structFieldError(s, structIndex, f.first, u8"Cannot override field in parent struct"));
                }
            }

            std::vector<StructField> fields;
            fields.reserve(parentFields.size() + s.fields.size());
            fields.insert(fields.end(), parentFields.begin(), parentFields.end());
            fields.insert(fields.end(), s.fields.begin(), s.fields.end());

            fieldMap.emplace(s.name, std::move(fields));
            return true;
        }
    };

    while (!toProcess.empty()) {
        auto it = std::remove_if(toProcess.begin(), toProcess.end(), processStruct);

        if (it == toProcess.end()) {
            for (const auto structIndex : toProcess) {
                const EntityRomStruct& s = structs.at(structIndex);

                if (s.parent != s.name) {
                    err.addError(entityRomStructError(s, structIndex, u8"Cannot find parent struct ", s.parent));
                }
            }
            return fieldMap;
        }
        toProcess.erase(it, toProcess.end());
    }

    return fieldMap;
}

// WARNING: output contains references.
// Output is only valid (and safe) so long as `functionTables` and `structFieldMap remain unchanged
// This function is static (local-only) because of the lifetime restrictions.
static FunctionTableMap generateFunctionTableFieldMap(const NamedList<EntityFunctionTable>& functionTables,
                                                      const StructFieldMap& structFieldMap,
                                                      const Project::ProjectFile& project, ErrorList& err)
{
    auto addError = [&](const EntityFunctionTable& ft, const unsigned ftIndex, const auto&... msg) {
        err.addError(entityFunctionTableError(ft, ftIndex, msg...));
    };

    static const FieldList blankFieldList;

    FunctionTableMap ftFieldMap(functionTables.size());

    for (auto [i, ft] : const_enumerate(functionTables)) {
        if (validate(ft, i, project, err) == false) {
            continue;
        }

        auto fieldsIt = structFieldMap.find(ft.entityStruct);
        if (fieldsIt == structFieldMap.end()) {
            addError(ft, i, u8"Unable to find entity struct for functionTable ", ft.entityStruct);
            continue;
        }

        auto s = ftFieldMap.emplace(ft.name, FunctionTableMap::mapped_type(ft, fieldsIt->second));
        if (s.second == false) {
            addError(ft, i, u8"Duplicate functionTable detected");
        }
    }

    return ftFieldMap;
}

static void writeIncFile_ListIds(StringStream& out, const std::vector<idstring>& listIds)
{
    out.write(u8"namespace EntityLists {\n"
              u8"\tcreateEnum()\n");

    for (auto& l : listIds) {
        out.write(u8"\t\tenum(", l, u8")\n");
    }

    out.write(u8"\tendEnum()\n"
              u8"}\n"
              u8"\n");
}

static void writeIncFile_StructField(StringStream& out, const StructField& f)
{
    out.write(u8"\t\tfield(", f.name, u8", ", fieldSize(f.type), u8") // ", fieldComment(f.type));
    if (!f.comment.empty()) {
        out.write(u8" ", f.comment);
    }
    out.write(u8"\n");
}

// Have to include the BaseRomStruct in the inc file to prevent a
// dependency error when assembling the project.
static void writeIncFile_BaseRomStruct(StringStream& out)
{
    out.write(u8"namespace " ENTITY_ROM_STRUCT_NAMESPACE u8"." BASE_ROM_STRUCT u8" {\n",
              u8"\tbasestruct_offset(", CompiledEntityRomData::ROM_DATA_LABEL, u8")\n");

    auto writeField = [&](std::u8string_view name, DataType type) {
        writeIncFile_StructField(out, StructField{ idstring::fromString(name), type, std::u8string{}, std::u8string{} });
    };

    // If you make any changes to this code you MUST ALSO UPDATE the
    // `processEntry` function.

    writeField(u8"defaultPalette", DataType::UINT8);
    writeField(u8"initialProjectileId", DataType::UINT8);
    writeField(u8"initialListId", DataType::UINT8);
    writeField(u8"frameSetId", DataType::UINT16);

    out.write(u8"\tendstruct()\n"
              u8"}\n");
}

static void writeIncFile_RomStruct(StringStream& out, const EntityRomStruct& s, bool hasChild)
{
    const idstring& parent = s.parent.isValid() ? s.parent : baseRomStruct;
    const char8_t* structType = hasChild ? u8"basestruct" : u8"childstruct";

    out.write(u8"namespace " ENTITY_ROM_STRUCT_NAMESPACE u8".", s.name, u8" {\n\t",
              structType, u8" (" ENTITY_ROM_STRUCT_NAMESPACE u8".", parent, u8")\n");

    for (const auto& f : s.fields) {
        writeIncFile_StructField(out, f);
    }
    out.write(u8"\tendstruct()\n"
              u8"}\n");
}

// assumes inputStructs are valid
static void writeIncFile_RomStructs(StringStream& out, const NamedList<EntityRomStruct>& structs)
{
    writeIncFile_BaseRomStruct(out);

    std::vector<const EntityRomStruct*> toProcess(structs.size());
    {
        auto it = toProcess.begin();
        for (const auto& s : structs) {
            *it++ = &s;
        }
        assert(it == toProcess.end());
    }

    std::unordered_set<idstring> processed;
    processed.reserve(structs.size());

    while (!toProcess.empty()) {
        auto it = std::remove_if(
            toProcess.begin(), toProcess.end(),
            [&](const EntityRomStruct* s) -> bool {
                bool parentProcessed = s->parent.isValid() == false
                                       || processed.find(s->parent) != processed.end();
                if (parentProcessed) {
                    bool hasChild = std::any_of(toProcess.begin(), toProcess.end(),
                                                [&](auto* c) { return c->parent == s->name; });
                    writeIncFile_RomStruct(out, *s, hasChild);
                    processed.insert(s->name);
                    return true;
                }
                else {
                    return false;
                }
            });

        toProcess.erase(it, toProcess.end());
    }

    out.write(u8"\n");
}

static const char8_t* prefixForEntityType(const EntityType entityType)
{
    switch (entityType) {
    case EntityType::ENTITY:
        return u8"Entities.";

    case EntityType::PROJECTILE:
        return u8"Entities.Projectiles.";

    case EntityType::PLAYER:
        return u8"Entities.Players.";
    }

    abort();
}

static void writeIncFile_FunctionTableDefines(StringStream& out, const NamedList<EntityFunctionTable>& functionTables)
{
    for (const auto& ft : functionTables) {
        const idstring& structName = ft.entityStruct.isValid() ? ft.entityStruct : baseRomStruct;
        out.write(u8"define ", prefixForEntityType(ft.entityType), ft.name, u8".RomStruct = " ENTITY_ROM_STRUCT_NAMESPACE u8".", structName,
                  u8"\ndefine ", prefixForEntityType(ft.entityType), ft.name, u8".ExportOrder = MSEO.", ft.exportOrder, u8"\n");
    }

    out.write(u8"\n");
}

static void writeIncFile_EntryIds(StringStream& out,
                                  const std::u8string& label, const NamedList<EntityRomEntry>& entries)
{
    out.write(u8"namespace ", label, u8" {\n",
              u8"\tconstant\tcount = ", entries.size(), u8"\n\n");

    for (auto [i, entry] : const_enumerate(entries)) {
        out.write(u8"\tconstant\t", entry.name, u8" = ", i, u8"\n");
    }

    out.write(u8"}\n"
              u8"\n");
}

static void writeIncFile_FunctionTableData(StringStream& out, const EntityType& entityType, const NamedList<EntityRomEntry>& entries)
{
    for (auto& entry : entries) {
        out.write(u8"\tdw\t", prefixForEntityType(entityType), entry.functionTable, u8".FunctionTable\n");
    }
}

// assumes entries are valid
static void processEntry(const EntityType entityType,
                         std::vector<uint8_t>& romData, const EntityRomEntry& entry,
                         const FunctionTableMap& ftMap, const Project::ProjectFile& project)
{
    const auto& frameSets = project.frameSets;
    const auto& projectiles = project.entityRomData.projectiles;
    const auto& listIds = project.entityRomData.listIds;

    unsigned frameSetId = std::find_if(frameSets.begin(), frameSets.end(),
                                       [&](auto& fs) { return fs.name() == entry.frameSetId; })
                          - frameSets.begin();

    unsigned initialProjectileId = std::min<unsigned>(0xff, projectiles.indexOf(entry.initialProjectileId));
    unsigned initialListId = entityType != EntityType::PLAYER ? std::find(listIds.begin(), listIds.end(), entry.initialListId) - listIds.begin()
                                                              : 0xff;

    assert(entry.defaultPalette <= UINT8_MAX);
    assert(initialProjectileId <= UINT8_MAX);
    assert(initialListId <= UINT8_MAX);
    assert(frameSetId <= UINT16_MAX);

    // If you make any changes to this code you MUST ALSO UPDATE the
    // `writeIncFile_BaseRomStruct` function.

    romData.push_back(entry.defaultPalette);
    romData.push_back(initialProjectileId);
    romData.push_back(initialListId);

    romData.push_back(frameSetId);
    romData.push_back(frameSetId >> 8);

    for (const StructField& field : ftMap.at(entry.functionTable).second) {
        auto it = entry.fields.find(field.name);
        const std::u8string& valueStr = it != entry.fields.end() ? it->second : field.defaultValue;

        auto writeUnsigned = [&](const unsigned length) {
            const auto value = String::decimalOrHexToUint32(valueStr);
            const uint32_t v = value.value_or(0);
            for (const auto i : range(length)) {
                romData.push_back(v >> (i * 8));
            }
        };
        auto writeSigned = [&](const unsigned length) {
            // confirm int32_t is two's complement
            static_assert(static_cast<uint32_t>(int32_t{ -1 }) == 0xffffffff);

            const auto value = String::decimalOrHexToInt32(valueStr);
            const uint64_t v = static_cast<uint32_t>(value.value_or(0));
            for (const auto i : range(length)) {
                romData.push_back(v >> (i * 8));
            }
        };

        switch (field.type) {
        case DataType::UINT8:
            writeUnsigned(1);
            break;

        case DataType::UINT16:
            writeUnsigned(2);
            break;

        case DataType::UINT24:
            writeUnsigned(3);
            break;

        case DataType::UINT32:
            writeUnsigned(4);
            break;

        case DataType::SINT8:
            writeSigned(1);
            break;

        case DataType::SINT16:
            writeSigned(2);
            break;

        case DataType::SINT24:
            writeSigned(3);
            break;

        case DataType::SINT32:
            writeSigned(4);
            break;
        }
    }
}

// assumes entries are valid
static void processRomData(const EntityType entityType,
                           std::vector<uint8_t>& indexes, std::vector<uint8_t>& romData,
                           const NamedList<EntityRomEntry>& entries,
                           const FunctionTableMap& ftMap, const Project::ProjectFile& project)
{
    for (const auto& entry : entries) {
        unsigned pos = romData.size();

        processEntry(entityType, romData, entry, ftMap, project);

        indexes.push_back(pos & 0xff);
        indexes.push_back((pos >> 8) & 0xff);
    }
}

const std::u8string CompiledEntityRomData::ROM_DATA_LIST_LABEL(u8"Project.EntityRomDataList");
const std::u8string CompiledEntityRomData::ROM_DATA_LABEL(u8"Project.EntityRomData");

std::shared_ptr<const CompiledEntityRomData>
compileEntityRomData(const EntityRomData& data, const Project::ProjectFile& project, ErrorList& err)
{
    const auto oldErrorCount = err.errorCount();

    if (data.entities.size() > MAX_N_ENTITY_ENTRIES) {
        err.addErrorString(u8"Too many entities (u8", data.entities.size(), u8", max: ", MAX_N_ENTITY_ENTRIES, u8")");
    }
    if (data.projectiles.size() > MAX_N_ENTITY_ENTRIES) {
        err.addErrorString(u8"Too many projectiles (u8", data.projectiles.size(), u8", max: ", MAX_N_ENTITY_ENTRIES, u8")");
    }

    auto ret = std::make_shared<CompiledEntityRomData>();

    validateListIds(data.listIds, err);

    const auto structFieldMap = generateStructMap(data.structs, err);
    const auto ftMap = generateFunctionTableFieldMap(data.functionTables, structFieldMap, project, err);

    for (auto [i, e] : const_enumerate(data.entities)) {
        validate(e, EntityType::ENTITY, i, project, ftMap, err);
    }
    for (auto [i, e] : const_enumerate(data.projectiles)) {
        validate(e, EntityType::PROJECTILE, i, project, ftMap, err);
    }
    for (auto [i, e] : const_enumerate(data.players)) {
        validate(e, EntityType::PLAYER, i, project, ftMap, err);
    }

    ret->valid = oldErrorCount == err.errorCount();
    if (!ret->valid) {
        return nullptr;
    }

    StringStream defines;
    StringStream functionTableData;

    writeIncFile_ListIds(defines, data.listIds);
    writeIncFile_EntryIds(defines, u8"Project.EntityIds", data.entities);
    writeIncFile_EntryIds(defines, u8"Project.ProjectileIds", data.projectiles);
    writeIncFile_EntryIds(defines, u8"Project.PlayerIds", data.players);
    writeIncFile_RomStructs(defines, data.structs);
    writeIncFile_FunctionTableDefines(defines, data.functionTables);

    functionTableData.write(u8"code()\n"
                            u8"Project.EntityFunctionTables:\n");
    writeIncFile_FunctionTableData(functionTableData, EntityType::ENTITY, data.entities);
    writeIncFile_FunctionTableData(functionTableData, EntityType::PROJECTILE, data.projectiles);
    writeIncFile_FunctionTableData(functionTableData, EntityType::PLAYER, data.players);

    functionTableData.write(u8"constant Project.EntityFunctionTables.size = pc() - Project.EntityFunctionTables"
                            u8"\n\n");

    const auto indexSize = (data.entities.size() + data.projectiles.size() + data.players.size()) * 2;
    ret->romDataIndexes.reserve(indexSize);

    processRomData(EntityType::ENTITY, ret->romDataIndexes, ret->romData, data.entities, ftMap, project);
    processRomData(EntityType::PROJECTILE, ret->romDataIndexes, ret->romData, data.projectiles, ftMap, project);
    processRomData(EntityType::PLAYER, ret->romDataIndexes, ret->romData, data.players, ftMap, project);

    assert(ret->romDataIndexes.size() == indexSize);

    ret->defines = defines.takeString();
    ret->functionTableData = functionTableData.takeString();

    for (auto [i, entity] : const_enumerate(data.entities)) {
        const auto it = ftMap.find(entity.functionTable);
        assert(it != ftMap.end());
        const EntityFunctionTable& ft = it->second.first;

        ret->entityNameMap.emplace(entity.name, std::make_pair(i, ft.parameterType));
    }

    return ret;
}

}
