/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entity-rom-data-editor.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"

namespace UnTech::Gui {

static const char* dataTypeItems[] = {
    "uint8",
    "uint16",
    "uint24",
    "uint32",
    "sint8",
    "sint16",
    "sint24",
    "sint32",
};

static const char* entityTypeItems[] = {
    "Entity",
    "Projectile",
    "Player",
};

using EntityRomStruct = UnTech::Entity::EntityRomStruct;

// EntityRomDataEditor Action Policies
struct EntityRomDataEditor::AP {
    struct EntityRomData {
        using EditorT = EntityRomDataEditor;
        using EditorDataT = UnTech::Entity::EntityRomData;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._entityRomData;
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

        constexpr static auto SelectionPtr = &EditorT::_listIdsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.listIds; }
    };

    struct Structs final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomStruct>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 255;

        constexpr static auto SelectionPtr = &EditorT::_structsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.structs; }
    };

    struct StructFields final : public EntityRomData {
        using ListT = std::vector<Entity::StructField>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = MultipleChildSelection;

        constexpr static size_t MAX_SIZE = 32;

        constexpr static auto SelectionPtr = &EditorT::_structFieldsSel;

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

        constexpr static auto SelectionPtr = &EditorT::_functionTablesSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.functionTables; }
    };

    struct Entities final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomEntry>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Entity::MAX_N_ENTITY_ENTRIES;

        constexpr static auto SelectionPtr = &EditorT::_entitiesSel;

        static constexpr EntityType entityType = EntityType::ENTITY;

        static ListT* getList(EditorDataT& editorData) { return &editorData.entities; }
    };

    struct Projectiles final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomEntry>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Entity::MAX_N_ENTITY_ENTRIES;

        constexpr static auto SelectionPtr = &EditorT::_projectilesSel;

        static constexpr EntityType entityType = EntityType::PROJECTILE;

        static ListT* getList(EditorDataT& editorData) { return &editorData.projectiles; }
    };

    struct Players final : public EntityRomData {
        using ListT = NamedList<Entity::EntityRomEntry>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Entity::MAX_N_ENTITY_ENTRIES;

        constexpr static auto SelectionPtr = &EditorT::_playersSel;

        static constexpr EntityType entityType = EntityType::PLAYER;

        static ListT* getList(EditorDataT& editorData) { return &editorData.players; }
    };
};

EntityRomDataEditor::EntityRomDataEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool EntityRomDataEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    _entityRomData = projectFile.entityRomData;

    return true;
}

void EntityRomDataEditor::editorOpened()
{
}

void EntityRomDataEditor::editorClosed()
{
}

void EntityRomDataEditor::listIdsWindow()
{
    if (ImGui::Begin("EntityListIds")) {
        ImGui::SetWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

        ListButtons<AP::ListIds>(this);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();

        for (unsigned i = 0; i < _entityRomData.listIds.size(); i++) {
            auto& listId = _entityRomData.listIds.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_listIdsSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &listId);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::ListIds>::itemEdited(this, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
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

static const char* toString(const UnTech::Entity::DataType type)
{
    unsigned i = static_cast<unsigned>(type);
    return i < IM_ARRAYSIZE(dataTypeItems) ? dataTypeItems[i] : "";
}

void EntityRomDataEditor::structsWindow()
{
    if (ImGui::Begin("Entity Rom Structs")) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        NamedListSidebar<AP::Structs>(this);

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::BeginChild("Item");
        {
            if (_structsSel.selected < _entityRomData.structs.size()) {
                auto& st = _entityRomData.structs.at(_structsSel.selected);

                {
                    bool edited = false;

                    ImGui::InputIdstring("Name", &st.name);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        ListActions<AP::Structs>::selectedFieldEdited<
                            &UnTech::Entity::EntityRomStruct::name>(this);
                    }

                    edited = parentCombo("Parent", &st.parent, _entityRomData.structs, _structsSel.selected);
                    if (edited) {
                        ListActions<AP::Structs>::selectedFieldEdited<
                            &UnTech::Entity::EntityRomStruct::parent>(this);
                    }

                    ImGui::InputText("Comment", &st.comment);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        ListActions<AP::Structs>::selectedFieldEdited<
                            &UnTech::Entity::EntityRomStruct::name>(this);
                    }
                }

                ImGui::Separator();

                {
                    ListButtons<AP::StructFields>(this);

                    ImGui::BeginChild("Scroll");

                    ImGui::Columns(5);
                    ImGui::SetColumnWidth(0, 40);

                    ImGui::Separator();
                    ImGui::NextColumn();
                    ImGui::Text("Name");
                    ImGui::NextColumn();
                    ImGui::Text("Type");
                    ImGui::NextColumn();
                    ImGui::Text("Default Value");
                    ImGui::NextColumn();
                    ImGui::Text("Comment");
                    ImGui::NextColumn();

                    const auto parentChain = generateStructChain(st.parent);

                    for (auto it = parentChain.rbegin(); it != parentChain.rend(); it++) {
                        const unsigned parentIndex = *it;
                        const auto& parent = _entityRomData.structs.at(parentIndex);

                        for (unsigned i = 0; i < parent.fields.size(); i++) {
                            auto& field = parent.fields.at(i);

                            ImGui::NextColumn();

                            ImGui::TextUnformatted(field.name);
                            ImGui::NextColumn();

                            ImGui::TextUnformatted(toString(field.type));
                            ImGui::NextColumn();

                            ImGui::TextUnformatted(field.defaultValue);
                            ImGui::NextColumn();

                            ImGui::TextUnformatted(field.comment);
                            ImGui::NextColumn();
                        }
                    }

                    for (unsigned i = 0; i < st.fields.size(); i++) {
                        auto& field = st.fields.at(i);

                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_structFieldsSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputIdstring("##Name", &field.name);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::EnumCombo("##Type", &field.type, dataTypeItems, IM_ARRAYSIZE(dataTypeItems));
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("##Default Value", &field.defaultValue);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputText("##Comment", &field.comment);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        if (edited) {
                            ListActions<AP::StructFields>::selectedListItemEdited(this, i);
                        }

                        ImGui::PopID();
                    }

                    ImGui::Columns(1);
                    ImGui::Separator();

                    ImGui::EndChild();
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
    ImGui::End();
}

void EntityRomDataEditor::functionTablesWindow(const UnTech::Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("Entity Function Tables")) {
        ImGui::SetWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);

        ListButtons<AP::FunctionTables>(this);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(7);
        //ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Entity Type");
        ImGui::NextColumn();
        ImGui::Text("Struct");
        ImGui::NextColumn();
        ImGui::Text("FS Export Order");
        ImGui::NextColumn();
        ImGui::Text("Parameter Type");
        ImGui::NextColumn();
        ImGui::Text("Comment");
        ImGui::NextColumn();
        ImGui::Separator();

        for (unsigned i = 0; i < _entityRomData.functionTables.size(); i++) {
            auto& ft = _entityRomData.functionTables.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_functionTablesSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &ft.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::EnumCombo("##EntityType", &ft.entityType, entityTypeItems, IM_ARRAYSIZE(entityTypeItems));
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##EntityStruct", &ft.entityStruct, _entityRomData.structs, true);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##ExportOrder", &ft.exportOrder, projectFile.frameSetExportOrders);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::EnumCombo("##ParameterType", &ft.parameterType, dataTypeItems, IM_ARRAYSIZE(dataTypeItems));
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##Comment", &ft.comment);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::FunctionTables>::itemEdited(this, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

template <typename ActionPolicy>
void EntityRomDataEditor::entityEntriesWindow(const char* name,
                                              const Project::ProjectFile& projectFile)
{
    using SelectionT = typename ActionPolicy::SelectionT;

    if (ImGui::Begin(name)) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        NamedListSidebar<ActionPolicy>(this);

        SelectionT& sel = this->*ActionPolicy::SelectionPtr;
        NamedList<EntityRomEntry>* list = ActionPolicy::getList(_entityRomData);

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::BeginChild("Item");
        {
            if (list && sel.selected < list->size()) {
                auto& entry = list->at(sel.selected);

                bool edited = false;

                ImGui::InputIdstring("Name", &entry.name);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                edited |= ImGui::IdStringCombo("Function Table", &entry.functionTable, _entityRomData.functionTables);

                const auto functionTable = _entityRomData.functionTables.find(entry.functionTable);

                ImGui::InputText("Comment", &entry.comment);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::Separator();

                edited |= ImGui::IdStringCombo("initialProjectileId", &entry.initialListId, _entityRomData.projectiles);

                if (ActionPolicy::entityType != EntityType::PLAYER) {
                    edited |= ImGui::IdStringCombo("initialListId", &entry.initialListId, _entityRomData.listIds, false);
                }

                edited |= ImGui::IdStringCombo("frameSetId", &entry.frameSetId, projectFile.frameSets, false,
                                               [](auto& fsf) { return &fsf.name(); });

                if (ImGui::BeginCombo("displayFrame", entry.displayFrame)) {
                    if (functionTable) {
                        if (auto* exportOrder = projectFile.frameSetExportOrders.find(functionTable->exportOrder)) {
                            edited |= ImGui::IdStringComboSelection(&entry.displayFrame, exportOrder->stillFrames, false,
                                                                    [](auto& f) { return &f.name; });
                        }
                    }
                    ImGui::End();
                }

                if (functionTable) {
                    const auto structChain = generateStructChain(functionTable->entityStruct);

                    ImGui::Separator();

                    unsigned id = 0;

                    for (auto it = structChain.rbegin(); it != structChain.rend(); it++) {
                        auto& st = _entityRomData.structs.at(*it);

                        for (unsigned i = 0; i < st.fields.size(); i++) {
                            auto& field = st.fields.at(i);

                            std::string& value = entry.fields[field.name];

                            ImGui::PushID(id++);

                            std::string label = stringBuilder(field.name, "###Field_", i);

                            ImGui::InputText(label.c_str(), &value);
                            edited |= ImGui::IsItemDeactivatedAfterEdit();
                            if (ImGui::IsItemHovered() && !field.comment.empty()) {
                                ImGui::BeginTooltip();
                                ImGui::TextUnformatted(field.comment);
                                ImGui::EndTooltip();
                            }
                            ImGui::NextColumn();

                            ImGui::PopID();
                        }
                    }
                }

                if (edited) {
                    ListActions<ActionPolicy>::selectedItemEdited(this);
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
    ImGui::End();
}

void EntityRomDataEditor::processGui(const Project::ProjectFile& projectFile)
{
    listIdsWindow();
    structsWindow();
    functionTablesWindow(projectFile);

    entityEntriesWindow<AP::Entities>("Entity ROM Entries", projectFile);

    entityEntriesWindow<AP::Projectiles>("Projectile ROM Entries", projectFile);

    entityEntriesWindow<AP::Players>("Player ROM Entries", projectFile);

    UpdateSelection(&_listIdsSel);

    UpdateSelection(&_structsSel);
    UpdateSelection(&_structFieldsSel, _structsSel);

    UpdateSelection(&_functionTablesSel);

    UpdateSelection(&_entitiesSel);
    UpdateSelection(&_projectilesSel);
    UpdateSelection(&_playersSel);
}

// ::TODO replace with array_vector::
std::vector<unsigned> EntityRomDataEditor::generateStructChain(const idstring& name) const
{
    std::vector<unsigned> items;
    items.reserve(8);

    unsigned sIndex = _entityRomData.structs.indexOf(name);
    while (sIndex < _entityRomData.structs.size()) {
        const bool containsParent = std::find(items.begin(), items.end(), sIndex) != items.end();
        if (containsParent) {
            items.clear();
            return items;
        }

        items.push_back(sIndex);

        const auto& st = _entityRomData.structs.at(sIndex);
        sIndex = _entityRomData.structs.indexOf(st.parent);
    }

    return items;
}

}
