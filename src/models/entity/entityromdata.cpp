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
#include "models/project/project.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace UnTech {
namespace Entity {

const int CompiledEntityRomData::ENTITY_FORMAT_VERSION = 6;

#define BASE_ROM_STRUCT "BaseEntityRomStruct"
#define ENTITY_ROM_STRUCT_NAMESPACE "Project.EntityRomStructs"
static const idstring baseRomStruct{ BASE_ROM_STRUCT };

const std::unordered_set<idstring> INVALID_NAMES{
    idstring{ "functionTable" },
    idstring{ "defaultPalette" },
    idstring{ "initialProjectileId" },
    idstring{ "initialListId" },
    idstring{ "frameSetId" },
    idstring{ "Players" },
    idstring{ "Projectiles" },
    idstring{ "size" },
    idstring{ "count" },
    idstring{ "__STRUCT__" },
    baseRomStruct,
};

static const char* entityTypeString(const EntityType entityType)
{
    switch (entityType) {
    case EntityType::ENTITY:
        return "Entity";

    case EntityType::PROJECTILE:
        return "Projectile";

    case EntityType::PLAYER:
        return "Player";
    }

    return "";
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

static const char* fieldComment(DataType type)
{
    switch (type) {
    case DataType::UINT8:
        return "uint8";

    case DataType::UINT16:
        return "uint16";

    case DataType::UINT24:
        return "uint24";

    case DataType::UINT32:
        return "uint32";

    case DataType::SINT8:
        return "sint8";

    case DataType::SINT16:
        return "sint16";

    case DataType::SINT24:
        return "sint24";

    case DataType::SINT32:
        return "sint32";
    }

    fputs("Invalid DataType\n", stderr);
    abort();
}

static bool validateFieldValue(DataType type, const std::string& str)
{
    auto testInteger = [&](long min, long max) {
        auto v = String::toLong(str);
        return v.exists()
               && v() >= min && v() <= max;
    };

    switch (type) {
    case DataType::UINT8:
        return testInteger(0, UINT8_MAX);

    case DataType::UINT16:
        return testInteger(0, UINT16_MAX);

    case DataType::UINT24:
        return testInteger(0, (1 << 24) - 1);

    case DataType::UINT32:
        return testInteger(0, UINT32_MAX);

    case DataType::SINT8:
        return testInteger(INT8_MIN, INT8_MAX);

    case DataType::SINT16:
        return testInteger(INT16_MIN, INT16_MAX);

    case DataType::SINT24:
        return testInteger(-(1 << 23), (1 << 23) - 1);

    case DataType::SINT32:
        return testInteger(INT32_MIN, INT32_MAX);
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
        addError("Missing field name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError("Invalid field name ", input.name);
    }
    if (input.comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    if (input.defaultValue.empty() == false) {
        if (!validateFieldValue(input.type, input.defaultValue)) {
            addError("Invalid defaultValue");
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
        addError("Expected name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError("Invalid name ", input.name);
    }
    if (input.name.str() == BASE_ROM_STRUCT) {
        addError("Name cannot be " BASE_ROM_STRUCT);
    }
    if (input.parent.isValid() && input.parent == input.name) {
        addError("Parent cannot refer to self");
    }
    if (input.fields.empty()) {
        addError("Expected at least one field");
    }
    if (input.comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    for (auto it = input.fields.cbegin(); it != input.fields.cend(); it++) {
        auto& field = *it;
        unsigned index = std::distance(input.fields.cbegin(), it);

        bool fieldValid = validate(field, input, structIndex, index, err);
        if (fieldValid) {
            auto dupIt = std::find_if(input.fields.begin(), it, [&](auto& f) { return f.name == field.name; });
            if (dupIt != it) {
                addStructFieldError(index, "Duplicate field name detected: ", field.name);
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
        addError("Missing EntityFunctionTable name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError("Invalid EntityFunctionTable name ", input.name);
    }
    if (input.exportOrder.isValid() == false) {
        addError("Missing export order");
    }
    if (input.comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    if (input.exportOrder.isValid()) {
        if (project.frameSetExportOrders.find(input.exportOrder) == nullptr) {
            addError("Cannot find FrameSet Export Order ", input.exportOrder);
        }
    }

    return valid;
}

static bool validate(const EntityRomEntry& input, const EntityType entityType, const unsigned index, const Project::ProjectFile& project, const FunctionTableMap& ftMap, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addError(entityRomEntryError(input, entityType, index, msg...));
        valid = false;
    };

    if (input.name.isValid() == false) {
        addError("Expected name");
    }
    if (INVALID_NAMES.find(input.name) != INVALID_NAMES.end()) {
        addError("Invalid name ", input.name);
    }
    if (input.functionTable.isValid() == false) {
        addError("Missing functionTable");
    }
    if (entityType != EntityType::PLAYER) {
        if (input.initialListId.isValid() == false) {
            addError("Missing initialListId");
        }
    }
    if (input.frameSetId.isValid() == false) {
        addError("Missing frameSetId");
    }
    if (input.displayFrame.isValid() == false) {
        addError("Missing displayFrame");
    }
    if (input.comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    if (input.initialProjectileId.isValid()) {
        const auto& projectiles = project.entityRomData.projectiles;
        if (not projectiles.find(input.initialProjectileId)) {
            addError("Unable to find projectile ", input.initialProjectileId);
        }
    }

    if (input.initialListId.isValid()) {
        auto& listIds = project.entityRomData.listIds;
        bool listIdValid = std::find(listIds.begin(), listIds.end(), input.initialListId) != listIds.end();
        if (!listIdValid) {
            addError("Unable to find listId ", input.initialListId);
        }
    }

    auto fieldsIt = ftMap.find(input.functionTable);
    if (fieldsIt != ftMap.end()) {
        const auto& ft = *fieldsIt->second.first;
        const auto& ftFields = *fieldsIt->second.second;

        if (ft.entityType != entityType) {
            addError("Function Table is the wrong type (expected ", entityTypeString(entityType), " but ", ft.name, " is a ", entityTypeString(ft.entityType), ")");
        }

        for (auto& field : ftFields) {
            auto fIt = input.fields.find(field.name);
            if (fIt != input.fields.end()) {
                const auto& value = fIt->second;
                if (value.empty()) {
                    if (field.defaultValue.empty()) {
                        addError("Missing field ", field.name);
                    }
                }
                else {
                    if (validateFieldValue(field.type, value) == false) {
                        addError("Invalid field ", field.name, ": ", value);
                    }
                }
            }
        }
    }
    else {
        addError("Unable to retrieve field list for functionTable ", input.functionTable);
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
                    const EntityFunctionTable& fTable = *fieldsIt->second.first;
                    if (fTable.exportOrder != frameSet.exportOrder) {
                        addError("export order for frameSet ", frameSet.name, " is not ", fTable.exportOrder);
                    }
                }
                if (input.displayFrame.isValid() && !frameSet.frames.find(input.displayFrame)) {
                    addError("Unable to find frame ", input.displayFrame);
                }
                if (input.defaultPalette >= nPalettes) {
                    addError("Invalid defaultPalette (value must be < ", nPalettes, ")");
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
                addError("Unable to read frameSet ", input.frameSetId);
            }
        }
        else {
            addError("Unable to find frameSet ", input.frameSetId);
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
        addError("Expected at least one Entity List Id");
    }

    for (auto [index, listId] : const_enumerate(listIds)) {
        if (listId.isValid() == false) {
            addListIdError(index, "Missing listId name");
            continue;
        }
        if (INVALID_NAMES.find(listId) != INVALID_NAMES.end()) {
            addListIdError(index, "Invalid ListId name ", listId);
            continue;
        }

        auto count = std::count(listIds.cbegin(), listIds.cbegin() + index, listId);
        if (count == 1) {
            addListIdError(index, "Duplicate listId detected: ", listId);
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

    auto processStruct = [&](const unsigned structIndex) -> bool {
        // returns true if the struct is to be removed from the process list
        const EntityRomStruct* s = &structs.at(structIndex);

        // Check name is unique and valid
        if (!s->name.isValid()) {
            return true;
        }
        auto it = fieldMap.find(s->name);
        if (it != fieldMap.end()) {
            err.addError(entityRomStructError(*s, structIndex, "Duplicate name detected"));
            return true;
        }

        if (s->parent.isValid() == false) {
            // no parent

            fieldMap.emplace(s->name, s->fields);
            return true;
        }
        else {
            auto parentIt = fieldMap.find(s->parent);
            if (parentIt == fieldMap.end()) {
                // parent hasn't been processed yet
                return false;
            }

            // struct has a parent and the parent has already been processed

            const auto& parentFields = parentIt->second;

            for (auto f : const_enumerate(s->fields)) {
                bool overridesField = std::any_of(parentFields.begin(), parentFields.end(), [&](const auto& o) { return f.second.name == o.name; });
                if (overridesField) {
                    err.addError(structFieldError(*s, structIndex, f.first, "Cannot override field in parent struct"));
                }
            }

            std::vector<StructField> fields;
            fields.reserve(parentFields.size() + s->fields.size());
            fields.insert(fields.end(), parentFields.begin(), parentFields.end());
            fields.insert(fields.end(), s->fields.begin(), s->fields.end());

            fieldMap.emplace(s->name, std::move(fields));
            return true;
        }
    };

    while (!toProcess.empty()) {
        auto it = std::remove_if(toProcess.begin(), toProcess.end(), processStruct);

        if (it == toProcess.end()) {
            for (const auto structIndex : toProcess) {
                const EntityRomStruct& s = structs.at(structIndex);

                if (s.parent != s.name) {
                    err.addError(entityRomStructError(s, structIndex, "Cannot find parent struct ", s.parent));
                }
            }
            return fieldMap;
        }
        toProcess.erase(it, toProcess.end());
    }

    return fieldMap;
}

// WARNING: output contains pointers.
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
            addError(ft, i, "Unable to find entity struct for functionTable ", ft.entityStruct);
            continue;
        }

        auto s = ftFieldMap.emplace(ft.name, std::make_pair(&ft, &fieldsIt->second));
        if (s.second == false) {
            addError(ft, i, "Duplicate functionTable detected");
        }
    }

    return ftFieldMap;
}

static void writeIncFile_ListIds(std::ostream& out, const std::vector<idstring>& listIds)
{
    out << "namespace EntityLists {\n"
           "\tcreateEnum()\n";
    for (auto& l : listIds) {
        out << "\t\tenum(" << l << ")\n";
    }
    out << "\tendEnum()\n"
           "}\n"
           "\n";
}

static void writeIncFile_StructField(std::ostream& out, const StructField& f)
{
    out << "\t\tfield(" << f.name << ", " << fieldSize(f.type) << ") // " << fieldComment(f.type);
    if (!f.comment.empty()) {
        out << ' ' << f.comment;
    }
    out << '\n';
}

// Have to include the BaseRomStruct in the inc file to prevent a
// dependency error when assembling the project.
static void writeIncFile_BaseRomStruct(std::ostream& out)
{
    out << "namespace " ENTITY_ROM_STRUCT_NAMESPACE "." BASE_ROM_STRUCT " {\n"
        << "\tbasestruct_offset(" << CompiledEntityRomData::ROM_DATA_LABEL << ")\n";

    auto writeField = [&](const char* name, DataType type) {
        writeIncFile_StructField(out, StructField{ idstring{ name }, type, idstring{}, std::string{} });
    };

    // If you make any changes to this code you MUST ALSO UPDATE the
    // `processEntry` function.

    writeField("defaultPalette", DataType::UINT8);
    writeField("initialProjectileId", DataType::UINT8);
    writeField("initialListId", DataType::UINT8);
    writeField("frameSetId", DataType::UINT16);

    out << "\tendstruct()\n"
           "}\n";
}

static void writeIncFile_RomStruct(std::ostream& out, const EntityRomStruct& s, bool hasChild)
{
    const idstring& parent = s.parent.isValid() ? s.parent : baseRomStruct;
    const char* structType = hasChild ? "basestruct" : "childstruct";

    out << "namespace " ENTITY_ROM_STRUCT_NAMESPACE "." << s.name << " {\n\t"
        << structType << " (" ENTITY_ROM_STRUCT_NAMESPACE "." << parent << ")\n";

    for (auto& f : s.fields) {
        writeIncFile_StructField(out, f);
    }
    out << "\tendstruct()\n"
           "}\n";
}

// assumes inputStructs are valid
static void writeIncFile_RomStructs(std::ostream& out, const NamedList<EntityRomStruct>& structs)
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

    out << "\n";
}

static const char* prefixForEntityType(const EntityType entityType)
{
    switch (entityType) {
    case EntityType::ENTITY:
        return "Entities.";

    case EntityType::PROJECTILE:
        return "Entities.Projectiles.";

    case EntityType::PLAYER:
        return "Entities.Players.";
    }

    abort();
}

static void writeIncFile_FunctionTableDefines(std::ostream& out, const NamedList<EntityFunctionTable>& functionTables)
{
    for (const auto& ft : functionTables) {
        const idstring& structName = ft.entityStruct.isValid() ? ft.entityStruct : baseRomStruct;
        out << "define " << prefixForEntityType(ft.entityType) << ft.name << ".RomStruct = " ENTITY_ROM_STRUCT_NAMESPACE "." << structName
            << "\ndefine " << prefixForEntityType(ft.entityType) << ft.name << ".ExportOrder = MSEO." << ft.exportOrder << '\n';
    }

    out << '\n';
}

static void writeIncFile_EntryIds(std::ostream& out,
                                  const std::string& label, const NamedList<EntityRomEntry>& entries)
{
    out << "namespace " << label << " {\n";
    out << "\tconstant\tcount = " << entries.size() << "\n\n";

    for (auto [i, entry] : const_enumerate(entries)) {
        out << "\tconstant\t" << entry.name << " = " << i << '\n';
    }

    out << "}\n"
           "\n";
}

static void writeIncFile_FunctionTableData(std::ostream& out, const EntityType& entityType, const NamedList<EntityRomEntry>& entries)
{
    for (auto& entry : entries) {
        out << "\tdw\t" << prefixForEntityType(entityType) << entry.functionTable << ".FunctionTable\n";
    }
}

// assumes entries are valid
static void processEntry(const EntityType entityType,
                         std::vector<uint8_t>& romData, const EntityRomEntry& entry,
                         const FunctionTableMap& ftMap, const Project::ProjectFile& project)
{
    auto writeValue = [&](const int64_t value, const unsigned length) {
        // confirm int64_t is two's complement
        static_assert(static_cast<uint64_t>(int64_t{ -1 }) == 0xffffffffffffffff);

        const uint64_t v = static_cast<uint64_t>(value);
        for (const auto i : range(length)) {
            romData.push_back(v >> (i * 8));
        }
    };

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
    writeValue(frameSetId, 2);

    for (auto& field : *ftMap.at(entry.functionTable).second) {
        auto it = entry.fields.find(field.name);
        const std::string& valueStr = it != entry.fields.end() ? it->second : field.defaultValue;
        const int64_t value = *String::toLong(valueStr);

        switch (field.type) {
        case DataType::UINT8:
        case DataType::SINT8:
            writeValue(value, 1);
            break;

        case DataType::UINT16:
        case DataType::SINT16:
            writeValue(value, 2);
            break;

        case DataType::UINT24:
        case DataType::SINT24:
            writeValue(value, 3);
            break;

        case DataType::UINT32:
        case DataType::SINT32:
            writeValue(value, 4);
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
    for (auto& entry : entries) {
        unsigned pos = romData.size();

        processEntry(entityType, romData, entry, ftMap, project);

        indexes.push_back(pos & 0xff);
        indexes.push_back((pos >> 8) & 0xff);
    }
}

const std::string CompiledEntityRomData::ROM_DATA_LIST_LABEL("Project.EntityRomDataList");
const std::string CompiledEntityRomData::ROM_DATA_LABEL("Project.EntityRomData");

std::shared_ptr<const CompiledEntityRomData>
compileEntityRomData(const EntityRomData& data, const Project::ProjectFile& project, ErrorList& err)
{
    const auto oldErrorCount = err.errorCount();

    if (data.entities.size() > MAX_N_ENTITY_ENTRIES) {
        err.addErrorString("Too many entities (", data.entities.size(), ", max: ", MAX_N_ENTITY_ENTRIES, ")");
    }
    if (data.projectiles.size() > MAX_N_ENTITY_ENTRIES) {
        err.addErrorString("Too many projectiles (", data.projectiles.size(), ", max: ", MAX_N_ENTITY_ENTRIES, ")");
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

    std::stringstream defines;
    std::stringstream functionTableData;

    writeIncFile_ListIds(defines, data.listIds);
    writeIncFile_EntryIds(defines, "Project.EntityIds", data.entities);
    writeIncFile_EntryIds(defines, "Project.ProjectileIds", data.projectiles);
    writeIncFile_EntryIds(defines, "Project.PlayerIds", data.players);
    writeIncFile_RomStructs(defines, data.structs);
    writeIncFile_FunctionTableDefines(defines, data.functionTables);

    functionTableData << "code()\n"
                         "Project.EntityFunctionTables:\n";
    writeIncFile_FunctionTableData(functionTableData, EntityType::ENTITY, data.entities);
    writeIncFile_FunctionTableData(functionTableData, EntityType::PROJECTILE, data.projectiles);
    writeIncFile_FunctionTableData(functionTableData, EntityType::PLAYER, data.players);
    functionTableData << "constant Project.EntityFunctionTables.size = pc() - Project.EntityFunctionTables"
                         "\n\n";

    const auto indexSize = (data.entities.size() + data.projectiles.size() + data.players.size()) * 2;
    ret->romDataIndexes.reserve(indexSize);

    processRomData(EntityType::ENTITY, ret->romDataIndexes, ret->romData, data.entities, ftMap, project);
    processRomData(EntityType::PROJECTILE, ret->romDataIndexes, ret->romData, data.projectiles, ftMap, project);
    processRomData(EntityType::PLAYER, ret->romDataIndexes, ret->romData, data.players, ftMap, project);

    assert(ret->romDataIndexes.size() == indexSize);

    ret->defines = defines.str();
    ret->functionTableData = functionTableData.str();

    for (auto [i, entity] : const_enumerate(data.entities)) {
        const auto it = ftMap.find(entity.functionTable);
        assert(it != ftMap.end());
        const EntityFunctionTable* ft = it->second.first;

        ret->entityNameMap.emplace(entity.name, std::make_pair(i, ft->parameterType));
    }

    return ret;
}

}
}
