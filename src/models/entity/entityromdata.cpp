/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entityromdata.h"
#include "models/common/string.h"
#include "models/project/project.h"
#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace UnTech {
namespace Entity {

const int CompiledEntityRomData::ENTITY_FORMAT_VERSION = 3;

#define BASE_ROM_STRUCT "BaseEntityRomStruct"
#define ENTITY_ROM_STRUCT_NAMESPACE "Project.EntityRomStructs"
static const idstring baseRomStruct{ BASE_ROM_STRUCT };

const std::unordered_set<idstring> INVALID_NAMES{
    idstring{ "functionTable" },
    idstring{ "defaultPalette" },
    idstring{ "initialProjectileId" },
    idstring{ "initialListId" },
    idstring{ "frameSetId" },
    idstring{ "size" },
    idstring{ "count" },
    idstring{ "__STRUCT__" },
    baseRomStruct,
};

static unsigned fieldSize(DataType type)
{
    switch (type) {
    case DataType::UINT8:
    case DataType::SINT8:
        return 1;

    case DataType::UINT16:
    case DataType::SINT16:
    case DataType::ADDR:
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

    case DataType::ADDR:
        return "addr";
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

    case DataType::ADDR:
        if (str.empty()) {
            return false;
        }
        for (const char& c : str) {
            if (c != '.' && !idstring::isCharValid(c)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

bool StructField::validate(ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](std::string s) {
        // ::TODO specialised error::
        err.addError(s);
        valid = false;
    };

    if (name.isValid() == false) {
        addError("Missing field name");
    }
    if (INVALID_NAMES.find(name) != INVALID_NAMES.end()) {
        addError("Invalid field name " + name);
    }
    if (comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    if (defaultValue.empty() == false) {
        if (!validateFieldValue(type, defaultValue)) {
            addError("Invalid defaultValue");
        }
    }

    return valid;
}

bool EntityRomStruct::validate(ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](std::string s) {
        // ::TODO specialised error::
        err.addError(s);
        valid = false;
    };

    if (name.isValid() == false) {
        addError("Expected name");
    }
    if (INVALID_NAMES.find(name) != INVALID_NAMES.end()) {
        addError("Invalid name " + name);
    }
    if (name.str() == BASE_ROM_STRUCT) {
        err.addError("Name cannot be " BASE_ROM_STRUCT);
    }
    if (fields.empty()) {
        addError("Expected at least one field");
    }
    if (comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    for (auto it = fields.cbegin(); it != fields.cend(); it++) {
        auto& field = *it;

        bool fieldValid = field.validate(err);
        if (fieldValid) {
            auto count = std::count_if(fields.begin(), it, [&](auto& f) { return f.name == field.name; });
            if (count == 1) {
                addError("Duplicate field name detected: " + field.name);
            }
        }
    }

    return valid;
}

bool EntityFunctionTable::validate(ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](std::string s) {
        // ::TODO specialised error::
        err.addError(s);
        valid = false;
    };

    if (name.isValid() == false) {
        addError("Missing EntityFunctionTable name");
    }
    if (INVALID_NAMES.find(name) != INVALID_NAMES.end()) {
        addError("Invalid EntityFunctionTable name " + name);
    }
    if (exportOrder.isValid() == false) {
        addError("Missing export order");
    }
    if (comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    return valid;
}

bool EntityRomEntry::validate(const Project::ProjectFile& project, const FunctionTableMap& ftMap, ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](std::string s) {
        // ::TODO specialised error::
        err.addError(s);
        valid = false;
    };

    if (name.isValid() == false) {
        addError("Expected name");
    }
    if (INVALID_NAMES.find(name) != INVALID_NAMES.end()) {
        addError("Invalid name " + name);
    }
    if (functionTable.isValid() == false) {
        addError("Missing functionTable");
    }
    if (initialListId.isValid() == false) {
        addError("Missing initialListId");
    }
    if (frameSetId.isValid() == false) {
        addError("Missing frameSetId");
    }
    if (displayFrame.isValid() == false) {
        addError("Missing displayFrame");
    }
    if (comment.find('\n') != std::string::npos) {
        addError("Comment must not contain a new line");
    }

    if (initialProjectileId.isValid()) {
        const auto& projectiles = project.entityRomData.projectiles;
        if (not projectiles.find(initialProjectileId)) {
            addError("Unable to find projectile " + initialProjectileId);
        }
    }

    if (initialListId.isValid()) {
        auto& listIds = project.entityRomData.listIds;
        bool listIdValid = std::find(listIds.begin(), listIds.end(), initialListId) != listIds.end();
        if (!listIdValid) {
            addError("Unable to find listId " + initialListId);
        }
    }

    auto fieldsIt = ftMap.find(functionTable);
    if (fieldsIt != ftMap.end()) {
        const auto& ftFields = *fieldsIt->second.second;

        for (auto& field : ftFields) {
            auto fIt = fields.find(field.name);
            if (fIt != fields.end()) {
                const auto& value = fIt->second;
                if (value.empty()) {
                    if (field.defaultValue.empty()) {
                        addError("Missing field " + field.name);
                    }
                }
                else {
                    if (validateFieldValue(field.type, value) == false) {
                        addError("Invalid field " + field.name + ": " + value);
                    }
                }
            }
        }
    }
    else {
        addError("Unable to retrieve field list for functionTable " + functionTable);
    }

    if (frameSetId.isValid()) {
        auto frameSetIt = std::find_if(project.frameSets.begin(), project.frameSets.end(),
                                       [&](auto& fs) { return fs.name() == frameSetId; });
        if (frameSetIt != project.frameSets.end()) {
            auto testFrameSet = [&](const auto& frameSet, unsigned nPalettes) {
                if (nPalettes == 0) {
                    nPalettes = 1;
                }

                if (fieldsIt != ftMap.end()) {
                    const EntityFunctionTable& fTable = *fieldsIt->second.first;
                    if (fTable.exportOrder != frameSet.exportOrder) {
                        addError("export order for frameSet " + frameSet.name + " is not " + fTable.exportOrder);
                    }
                }
                if (displayFrame.isValid() && !frameSet.frames.find(displayFrame)) {
                    addError("Unable to find frame " + displayFrame);
                }
                if (defaultPalette >= nPalettes) {
                    addError("Invalid defaultPalette (value must be < " + std::to_string(nPalettes) + ")");
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
                addError("Unable to read frameSet " + frameSetId);
            }
        }
        else {
            addError("Unable to find frameSet " + frameSetId);
        }
    }

    return valid;
}

bool EntityRomData::validateListIds(ErrorList& err) const
{
    bool valid = true;
    auto addError = [&](std::string s) {
        err.addError(s);
        valid = false;
    };

    if (listIds.empty()) {
        addError("Expected at least one Entity List Id");
    }

    for (auto it = listIds.cbegin(); it != listIds.cend(); it++) {
        auto& listId = *it;

        if (listId.isValid() == false) {
            addError("Missing listId name");
            continue;
        }
        if (INVALID_NAMES.find(listId) != INVALID_NAMES.end()) {
            addError("Invalid ListId name " + listId);
            continue;
        }

        auto count = std::count(listIds.cbegin(), it, listId);
        if (count == 1) {
            addError("Duplicate listId detected: " + listId);
        }
    }

    return valid;
}

StructFieldMap generateStructMap(const NamedList<EntityRomStruct>& structs, ErrorList& err)
{
    std::vector<const EntityRomStruct*> toProcess(structs.size());
    {
        auto it = toProcess.begin();
        for (const auto& s : structs) {
            *it++ = &s;
        }
        assert(it == toProcess.end());
    }

    StructFieldMap fieldMap(toProcess.size() + 1);
    fieldMap.emplace(idstring(), FieldList());

    auto processStruct = [&](const EntityRomStruct* s) -> bool {
        // returns true if the struct is to be removed from the process list
        assert(s);

        s->validate(err);

        // Check name is unique and valid
        if (!s->name.isValid()) {
            return true;
        }
        auto it = fieldMap.find(s->name);
        if (it != fieldMap.end()) {
            err.addError("struct " + s->name + " already exists");
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

            const auto& parentFields = parentIt->second;

            s->validate(err);

            for (const auto& f : s->fields) {
                bool overridesField = std::any_of(parentFields.begin(), parentFields.end(), [&](const auto& o) { return f.name == o.name; });
                if (overridesField) {
                    // ::TODO specialised error::
                    err.addError("Cannot override field in parent struct: " + f.name);
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
            for (auto* f : toProcess) {
                // ::TODO specialised error::
                err.addError("Cannot find parent for struct " + f->name);
            }
            return fieldMap;
        }
        toProcess.erase(it, toProcess.end());
    }

    return fieldMap;
}

FunctionTableMap generateFunctionTableFieldMap(const NamedList<EntityFunctionTable>& functionTables,
                                               const StructFieldMap& structFieldMap, ErrorList& err)
{
    static const FieldList blankFieldList;

    FunctionTableMap ftFieldMap(functionTables.size());

    for (const auto& ft : functionTables) {
        if (ft.validate(err) == false) {
            continue;
        }

        auto fieldsIt = structFieldMap.find(ft.entityStruct);
        if (fieldsIt == structFieldMap.end()) {
            err.addError("Unable to find entity struct for functionTable " + ft.name);
            continue;
        }

        auto s = ftFieldMap.emplace(ft.name, std::make_pair(&ft, &fieldsIt->second));
        if (s.second == false) {
            err.addError("Duplicate functionTable detected: " + ft.name);
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
        << "\tbasestruct_offset(Project.EntityRomData)\n";

    auto writeField = [&](const char* name, DataType type) {
        writeIncFile_StructField(out, StructField{ idstring{ name }, type, idstring{}, std::string{} });
    };

    writeField("functionTable", DataType::ADDR);
    writeField("initialListId", DataType::UINT8);
    writeField("defaultPalette", DataType::UINT8);
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

static void writeIncFile_FunctionTables(std::ostream& out, const NamedList<EntityFunctionTable>& functionTables)
{
    for (const auto& ft : functionTables) {
        const idstring& structName = ft.name.isValid() ? ft.entityStruct : baseRomStruct;
        out << "define Entities." << ft.name << ".RomStruct = " ENTITY_ROM_STRUCT_NAMESPACE "." << structName
            << "\ndefine Entities." << ft.name << ".ExportOrder = MSEO." << ft.exportOrder << '\n';
    }

    out << '\n';
}

static void writeIncFile_EntryIds(std::ostream& out,
                                  const std::string& label, const NamedList<EntityRomEntry>& entries)
{
    out << "namespace " << label << " {\n";
    out << "\tconstant\tcount = " << entries.size() << "\n\n";

    for (unsigned i = 0; i < entries.size(); i++) {
        out << "\tconstant\t" << entries.at(i).name << " = " << i << '\n';
    }

    out << "}\n"
           "\n";
}

// assumes entries are valid
// returns size of entry
static unsigned processEntry(std::ostream& out, const EntityRomEntry& entry,
                             const FunctionTableMap& ftMap, const Project::ProjectFile& project)
{
    char previousType = '\0';
    auto writeValue = [&](const char type, const auto& value) {
        if (type != previousType) {
            previousType = type;
            out << "\n\td" << type << '\t';
        }
        else {
            out << ", ";
        }
        out << value;
    };

    const auto& frameSets = project.frameSets;
    const auto& projectiles = project.entityRomData.projectiles;
    const auto& listIds = project.entityRomData.listIds;

    unsigned frameSetId = std::find_if(frameSets.begin(), frameSets.end(),
                                       [&](auto& fs) { return fs.name() == entry.frameSetId; })
                          - frameSets.begin();

    unsigned initialProjectileId = std::min<unsigned>(0xff, projectiles.indexOf(entry.initialProjectileId));
    unsigned initialListId = std::find(listIds.begin(), listIds.end(), entry.initialListId) - listIds.begin();

    assert(entry.defaultPalette <= UINT8_MAX);
    assert(initialProjectileId <= UINT8_MAX);
    assert(initialListId <= UINT8_MAX);
    assert(frameSetId <= UINT16_MAX);

    out << "\tdw\tEntities." << entry.functionTable << ".FunctionTable\n"
        << "\tdb\t" << entry.defaultPalette << ", " << initialProjectileId << ", " << initialListId << '\n'
        << "\tdw\t" << frameSetId;

    unsigned size = 7;
    previousType = 'w';

    for (auto& field : *ftMap.at(entry.functionTable).second) {
        auto it = entry.fields.find(field.name);
        const std::string& value = it != entry.fields.end() ? it->second : field.defaultValue;

        switch (field.type) {
        case DataType::UINT8:
        case DataType::SINT8:
            writeValue('b', value);
            size += 1;
            break;

        case DataType::UINT16:
        case DataType::SINT16:
        case DataType::ADDR:
            writeValue('w', value);
            size += 2;
            break;

        case DataType::UINT24:
        case DataType::SINT24:
            writeValue('l', value);
            size += 3;
            break;

        case DataType::UINT32:
        case DataType::SINT32:
            writeValue('d', value);
            size += 4;
            break;
        }
    }

    out << '\n';

    return size;
}

// assumes entries are valid
static unsigned processEntries(std::ostream& out, std::vector<uint8_t>& indexes,
                               const NamedList<EntityRomEntry>& entries, unsigned startingOffset,
                               const FunctionTableMap& ftMap, const Project::ProjectFile& project)
{
    assert(indexes.empty());
    indexes.reserve(entries.size() * 2);

    unsigned pos = startingOffset;
    for (auto& entry : entries) {
        unsigned size = processEntry(out, entry, ftMap, project);

        indexes.push_back(pos & 0xff);
        indexes.push_back((pos >> 8) & 0xff);

        pos += size;
    }

    return pos;
}

const std::string CompiledEntityRomData::ENTITY_INDEXES_LABEL("Project.EntityRomDataList");
const std::string CompiledEntityRomData::PROJECTILE_INDEXES_LABEL("Project.ProjectileRomDataList");

CompiledEntityRomData compileEntityRomData(const EntityRomData& data, const Project::ProjectFile& project, ErrorList& err)
{
    CompiledEntityRomData ret;

    auto oldErrorCount = err.errorCount();

    data.validateListIds(err);

    const auto structFieldMap = generateStructMap(data.structs, err);
    const auto ftMap = generateFunctionTableFieldMap(data.functionTables, structFieldMap, err);

    for (const auto& e : data.entities) {
        e.validate(project, ftMap, err);
    }
    for (const auto& e : data.projectiles) {
        e.validate(project, ftMap, err);
    }

    ret.valid = oldErrorCount == err.errorCount();
    if (!ret.valid) {
        return ret;
    }

    std::stringstream defines;
    std::stringstream entries;

    writeIncFile_ListIds(defines, data.listIds);
    writeIncFile_EntryIds(defines, "Project.EntityIds", data.entities);
    writeIncFile_EntryIds(defines, "Project.ProjectileIds", data.projectiles);
    writeIncFile_RomStructs(defines, data.structs);
    writeIncFile_FunctionTables(defines, data.functionTables);

    entries << "Project.EntityRomData:\n";
    unsigned startingIndex = processEntries(entries, ret.entityIndexes, data.entities, 0, ftMap, project);
    processEntries(entries, ret.projectileIndexes, data.projectiles, startingIndex, ftMap, project);
    entries << '\n';

    ret.defines = defines.str();
    ret.entries = entries.str();

    return ret;
}

}
}
