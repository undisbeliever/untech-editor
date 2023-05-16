/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entity-rom-data-editor.h"
#include "gui/aptable.h"
#include "gui/splitter.hpp"
#include "models/common/iterators.h"
#include "models/entity/entityromdata-error.h"

namespace UnTech::Gui {

using EntityRomStruct = UnTech::Entity::EntityRomStruct;

// EntityRomDataEditor Action Policies
struct EntityRomDataEditorData::AP {
    struct EntityRomData {
        using EditorT = EntityRomDataEditorData;
        using EditorDataT = UnTech::Entity::EntityRomData;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.entityRomData;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.entityRomData;
        }
    };

    struct ListIds final : public EntityRomData {
        using ListT = std::vector<idstring>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::listIdsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.listIds; }
    };

    struct Structs final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomStruct>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 255;

        constexpr static auto SelectionPtr = &EditorT::structsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.structs; }
    };

    struct StructFields final : public EntityRomData {
        using ListT = std::vector<Entity::StructField>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = 32;

        constexpr static auto SelectionPtr = &EditorT::structFieldsSel;

        static ListT* getList(EditorDataT& editorData, unsigned structIndex)
        {
            return getListField(Structs::getList(editorData), structIndex,
                                &EntityRomStruct::fields);
        }
    };

    struct FunctionTables final : public EntityRomData {
        using ListT = NamedList<Entity::EntityFunctionTable>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 255;

        constexpr static auto SelectionPtr = &EditorT::functionTablesSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.functionTables; }
    };

    struct Entities final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomEntry>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Entity::MAX_N_ENTITY_ENTRIES;

        constexpr static auto SelectionPtr = &EditorT::entitiesSel;

        static constexpr EntityType entityType = EntityType::ENTITY;

        static ListT* getList(EditorDataT& editorData) { return &editorData.entities; }
    };

    struct Projectiles final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomEntry>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Entity::MAX_N_ENTITY_ENTRIES;

        constexpr static auto SelectionPtr = &EditorT::projectilesSel;

        static constexpr EntityType entityType = EntityType::PROJECTILE;

        static ListT* getList(EditorDataT& editorData) { return &editorData.projectiles; }
    };

    struct Players final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomEntry>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Entity::MAX_N_ENTITY_ENTRIES;

        constexpr static auto SelectionPtr = &EditorT::playersSel;

        static constexpr EntityType entityType = EntityType::PLAYER;

        static ListT* getList(EditorDataT& editorData) { return &editorData.players; }
    };
};

EntityRomDataEditorData::EntityRomDataEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
{
}

bool EntityRomDataEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    entityRomData = projectFile.entityRomData;

    return true;
}

void EntityRomDataEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = Entity::EntityErrorType;

    listIdsSel.clearSelection();
    structsSel.clearSelection();
    structFieldsSel.clearSelection();
    functionTablesSel.clearSelection();
    entitiesSel.clearSelection();
    projectilesSel.clearSelection();
    playersSel.clearSelection();

    if (auto* e = dynamic_cast<const Entity::EntityError*>(error)) {
        switch (e->type) {
        case Type::LIST_ID:
            listIdsSel.setSelected(e->firstIndex);
            break;

        case Type::STRUCT:
            structsSel.setSelected(e->firstIndex);
            break;

        case Type::STRUCT_FIELD:
            structsSel.setSelected(e->firstIndex);
            structFieldsSel.setSelected(e->firstIndex, e->childIndex);
            break;

        case Type::ENTITY_FUNCTION_TABLE:
            functionTablesSel.setSelected(e->firstIndex);
            break;

        case Type::ENTITY_ROM_ENTRY:
            entitiesSel.setSelected(e->firstIndex);
            break;

        case Type::PROJECTILE_ROM_ENTRY:
            projectilesSel.setSelected(e->firstIndex);
            break;

        case Type::PLAYER_ROM_ENTRY:
            playersSel.setSelected(e->firstIndex);
            break;
        }
    }
}

void EntityRomDataEditorData::updateSelection()
{
    listIdsSel.update();
    structsSel.update();
    structFieldsSel.update(structsSel);
    functionTablesSel.update();
    entitiesSel.update();
    projectilesSel.update();
    playersSel.update();
}

EntityRomDataEditorGui::EntityRomDataEditorGui()
    : AbstractEditorGui("##Entity Rom Data Editor")
    , _data(nullptr)
    , _sidebar{ 575, 400, 500 }
    , _leftTopbar{ 400, 200, 200 }
    , _listIdsSidebar{ 200, 120, 350 }
    , _playersTopbar{ 250, 150, 320 }
    , _projectilesTopbar{ 250, 150, 150 }
{
}

bool EntityRomDataEditorGui::setEditorData(const std::shared_ptr<AbstractEditorData>& data)
{
    _data = std::dynamic_pointer_cast<EntityRomDataEditorData>(data);
    return _data != nullptr;
}

void EntityRomDataEditorGui::resetState()
{
}

void EntityRomDataEditorGui::editorClosed()
{
}

void EntityRomDataEditorGui::listIdsGui()
{
    assert(_data);

    ImGui::TextUnformatted(u8"ListIds:");

    apTable<AP::ListIds>(
        "ListIds", _data,
        std::to_array({ "Name" }),

        [&](auto& listId) { return Cell("##name", &listId); });
}

static bool parentCombo(const char* label, idstring* value, const NamedList<EntityRomStruct>& structs,
                        const unsigned structIndex)
{
    const idstring& currentStructName = structs.at(structIndex).name;

    return ImGui::IdStringCombo(label, value, structs, true,
                                [&](const EntityRomStruct& st) {
                                    return (st.name != currentStructName) ? &st.name : nullptr;
                                });
}

void EntityRomDataEditorGui::structsGui()
{
    assert(_data);
    auto& entityRomData = _data->entityRomData;

    ImGui::TextUnformatted(u8"Entity Structs:");

    NamedListSidebar<AP::Structs>(_data);

    ImGui::SameLine();

    ImGui::BeginChild("Item");
    {
        if (_data->structsSel.selectedIndex() < entityRomData.structs.size()) {
            auto& st = entityRomData.structs.at(_data->structsSel.selectedIndex());

            {
                if (Cell("Name", &st.name)) {
                    ListActions<AP::Structs>::selectedFieldEdited<
                        &UnTech::Entity::EntityRomStruct::name>(_data);
                }

                if (parentCombo("Parent", &st.parent, entityRomData.structs, _data->structsSel.selectedIndex())) {
                    ListActions<AP::Structs>::selectedFieldEdited<
                        &UnTech::Entity::EntityRomStruct::parent>(_data);
                }

                if (Cell("Comment", &st.comment)) {
                    ListActions<AP::Structs>::selectedFieldEdited<
                        &UnTech::Entity::EntityRomStruct::name>(_data);
                }
            }

            ImGui::Separator();

            ListButtons<AP::StructFields>(_data);

            constexpr auto columnNames = std::to_array({ "Name", "Type", "Default Value", "Comment" });
            if (beginApTable("Table", columnNames)) {
                const auto parentChain = generateStructChain(st.parent);

                for (const auto& parentIndex : reverse(parentChain)) {
                    const auto& parent = entityRomData.structs.at(parentIndex);

                    for (auto [i, field] : enumerate(parent.fields)) {
                        ImGui::TableNextColumn();

                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(field.name);

                        ImGui::TableNextColumn();
                        ImGui::TextEnum(field.type);

                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(field.defaultValue);

                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(field.comment);
                    }
                }

                apTable_data<AP::StructFields>(
                    _data,
                    [&](auto& sf) { return Cell("##Name", &sf.name); },
                    [&](auto& sf) { return Cell("##Type", &sf.type); },
                    [&](auto& sf) { return Cell("##Default Value", &sf.defaultValue); },
                    [&](auto& sf) { return Cell("##Comment", &sf.comment); });

                ImGui::EndTable();
            }
        }
    }
    ImGui::EndChild();
}

void EntityRomDataEditorGui::functionTablesGui(const UnTech::Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& entityRomData = _data->entityRomData;

    ImGui::TextUnformatted(u8"Entity Function Tables:");

    apTable<AP::FunctionTables>(
        "Table", _data,
        std::to_array({ "Name", "EntityType", "EntityStruct", "FS Export Order", "Parameter Type", "Comment" }),

        [&](auto& ft) { return Cell("##Name", &ft.name); },
        [&](auto& ft) { return Cell("##EntityType", &ft.entityType); },
        [&](auto& ft) { return Cell("##EntityStruct", &ft.entityStruct, entityRomData.structs, true); },
        [&](auto& ft) { return Cell("##ExportOrder", &ft.exportOrder, projectFile.frameSetExportOrders); },
        [&](auto& ft) { return Cell("##ParameterType", &ft.parameterType); },
        [&](auto& ft) { return Cell("##Comment", &ft.comment); });
}

template <typename ActionPolicy>
void EntityRomDataEditorGui::entityEntriesGui(const char8_t* text, const Project::ProjectFile& projectFile)
{
    using SelectionT = typename ActionPolicy::SelectionT;
    using EntityRomEntry = UnTech::Entity::EntityRomEntry;
    using EntityType = UnTech::Entity::EntityType;

    assert(_data);
    auto& entityRomData = _data->entityRomData;

    ImGui::TextUnformatted(text);

    NamedListSidebar<ActionPolicy>(_data);

    SelectionT& sel = (*_data).*(ActionPolicy::SelectionPtr);
    NamedList<EntityRomEntry>* list = ActionPolicy::getList(entityRomData);

    ImGui::SameLine();

    ImGui::BeginChild("Item");
    {
        if (list && sel.selectedIndex() < list->size()) {
            auto& entry = list->at(sel.selectedIndex());

            bool edited = false;

            edited |= Cell("Name", &entry.name);
            edited |= Cell("Function Table", &entry.functionTable, entityRomData.functionTables);
            edited |= Cell("Comment", &entry.comment);

            ImGui::Separator();

            edited |= Cell("initialProjectileId", &entry.initialProjectileId, entityRomData.projectiles);

            if (ActionPolicy::entityType != EntityType::PLAYER) {
                edited |= Cell("initialListId", &entry.initialListId, entityRomData.listIds, false);
            }

            edited |= ImGui::IdStringCombo("frameSetId", &entry.frameSetId, projectFile.frameSets, false,
                                           [](auto& fsf) { return &fsf.name(); });

            if (ImGui::BeginCombo("displayFrame", u8Cast(entry.displayFrame))) {
                auto frameSetIt = std::find_if(projectFile.frameSets.begin(), projectFile.frameSets.end(),
                                               [&](auto& fs) { return fs.name() == entry.frameSetId; });

                if (frameSetIt != projectFile.frameSets.end()) {
                    auto& fs = *frameSetIt;

                    if (fs.siFrameSet) {
                        edited |= ImGui::IdStringComboSelection(&entry.displayFrame, fs.siFrameSet->frames, false,
                                                                [](auto& f) { return &f.name; });
                    }
                    else if (fs.msFrameSet) {
                        edited |= ImGui::IdStringComboSelection(&entry.displayFrame, fs.msFrameSet->frames, false,
                                                                [](auto& f) { return &f.name; });
                    }
                }

                ImGui::EndCombo();
            }

            const auto functionTable = entityRomData.functionTables.find(entry.functionTable);
            if (functionTable) {
                const auto structChain = generateStructChain(functionTable->entityStruct);

                ImGui::Separator();

                unsigned id = 0;

                for (const auto& sIndex : reverse(structChain)) {
                    auto& st = entityRomData.structs.at(sIndex);

                    for (auto [i, field] : enumerate(st.fields)) {
                        std::u8string& value = entry.fields[field.name];

                        ImGui::PushID(id++);

                        std::u8string label = stringBuilder(field.name, u8"###Field_", i);

                        edited |= Cell(u8Cast(label), &value);

                        if (ImGui::IsItemHovered() && !field.comment.empty()) {
                            ImGui::ShowTooltip(field.comment);
                        }

                        ImGui::PopID();
                    }
                }
            }

            if (edited) {
                ListActions<ActionPolicy>::selectedItemEdited(_data);
            }
        }
    }
    ImGui::EndChild();
}

void EntityRomDataEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    splitterSidebarRight(
        "##splitter", &_sidebar,
        "##Content",
        [&] {
            splitterTopbar(
                "##ltsplitter", &_leftTopbar,
                "##Top",
                [&] {
                    splitterSidebarLeft(
                        "##listSplitter", &_listIdsSidebar,
                        "##ListIds",
                        [&]() {
                            listIdsGui();
                        },
                        "##Structs",
                        [&]() {
                            structsGui();
                        });
                },
                "##Bottom",
                [&] {
                    functionTablesGui(projectFile);
                });
        },
        "##Sidebar",
        [&] {
            splitterTopbar3(
                "##esplitOne", "##esplitTwo", &_playersTopbar, &_projectilesTopbar,
                "##Players",
                [&] {
                    entityEntriesGui<AP::Players>(u8"Players:", projectFile);
                },
                "##Projectiles",
                [&] {
                    entityEntriesGui<AP::Projectiles>(u8"Projectiles:", projectFile);
                },
                "##Entities",
                [&] {
                    entityEntriesGui<AP::Entities>(u8"Entities:", projectFile);
                });
        });
}

// ::TODO replace with array_vector::
std::vector<unsigned> EntityRomDataEditorGui::generateStructChain(const idstring& name) const
{
    assert(_data);
    const auto& entityRomData = _data->entityRomData;

    std::vector<unsigned> items;
    items.reserve(8);

    auto sIndex = entityRomData.structs.indexOf(name);
    while (sIndex) {
        const bool containsParent = std::find(items.begin(), items.end(), sIndex) != items.end();
        if (containsParent) {
            items.clear();
            return items;
        }

        items.push_back(sIndex.value());

        const auto& st = entityRomData.structs.at(sIndex.value());
        sIndex = entityRomData.structs.indexOf(st.parent);
    }

    return items;
}

}
