/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entity-rom-data-editor.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "models/common/iterators.h"

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
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool EntityRomDataEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<EntityRomDataEditorData*>(data));
}

void EntityRomDataEditorGui::editorDataChanged()
{
}

void EntityRomDataEditorGui::editorOpened()
{
}

void EntityRomDataEditorGui::editorClosed()
{
}

void EntityRomDataEditorGui::listIdsWindow()
{
    assert(_data);
    auto& entityRomData = _data->entityRomData;

    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("EntityListIds")) {

        ListButtons<AP::ListIds>(_data);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 40);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();

        for (auto [i, listId] : enumerate(entityRomData.listIds)) {
            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->listIdsSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &listId);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::ListIds>::itemEdited(_data, i);
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

void EntityRomDataEditorGui::structsWindow()
{
    assert(_data);
    auto& entityRomData = _data->entityRomData;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Entity Rom Structs")) {

        NamedListSidebar<AP::Structs>(_data);

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::BeginChild("Item");
        {
            if (_data->structsSel.selectedIndex() < entityRomData.structs.size()) {
                auto& st = entityRomData.structs.at(_data->structsSel.selectedIndex());

                {
                    bool edited = false;

                    ImGui::InputIdstring("Name", &st.name);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        ListActions<AP::Structs>::selectedFieldEdited<
                            &UnTech::Entity::EntityRomStruct::name>(_data);
                    }

                    edited = parentCombo("Parent", &st.parent, entityRomData.structs, _data->structsSel.selectedIndex());
                    if (edited) {
                        ListActions<AP::Structs>::selectedFieldEdited<
                            &UnTech::Entity::EntityRomStruct::parent>(_data);
                    }

                    ImGui::InputText("Comment", &st.comment);
                    if (ImGui::IsItemDeactivatedAfterEdit()) {
                        ListActions<AP::Structs>::selectedFieldEdited<
                            &UnTech::Entity::EntityRomStruct::name>(_data);
                    }
                }

                ImGui::Separator();

                {
                    ListButtons<AP::StructFields>(_data);

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
                        const auto& parent = entityRomData.structs.at(parentIndex);

                        for (auto [i, field] : enumerate(parent.fields)) {
                            ImGui::NextColumn();

                            ImGui::TextUnformatted(field.name);
                            ImGui::NextColumn();

                            ImGui::TextEnum(field.type);
                            ImGui::NextColumn();

                            ImGui::TextUnformatted(field.defaultValue);
                            ImGui::NextColumn();

                            ImGui::TextUnformatted(field.comment);
                            ImGui::NextColumn();
                        }
                    }

                    for (auto [i, field] : enumerate(st.fields)) {
                        bool edited = false;

                        ImGui::PushID(i);

                        ImGui::Selectable(&_data->structFieldsSel, i);
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        ImGui::InputIdstring("##Name", &field.name);
                        edited |= ImGui::IsItemDeactivatedAfterEdit();
                        ImGui::NextColumn();

                        ImGui::SetNextItemWidth(-1);
                        edited |= ImGui::EnumCombo("##Type", &field.type);
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
                            ListActions<AP::StructFields>::selectedListItemEdited(_data, i);
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

void EntityRomDataEditorGui::functionTablesWindow(const UnTech::Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& entityRomData = _data->entityRomData;

    ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Entity Function Tables")) {

        ListButtons<AP::FunctionTables>(_data);

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

        for (auto [i, ft] : enumerate(entityRomData.functionTables)) {
            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->functionTablesSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##Name", &ft.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::EnumCombo("##EntityType", &ft.entityType);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##EntityStruct", &ft.entityStruct, entityRomData.structs, true);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##ExportOrder", &ft.exportOrder, projectFile.frameSetExportOrders);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::EnumCombo("##ParameterType", &ft.parameterType);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputText("##Comment", &ft.comment);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::FunctionTables>::itemEdited(_data, i);
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
void EntityRomDataEditorGui::entityEntriesWindow(const char* name,
                                                 const Project::ProjectFile& projectFile)
{
    using SelectionT = typename ActionPolicy::SelectionT;
    using EntityRomEntry = UnTech::Entity::EntityRomEntry;
    using EntityType = UnTech::Entity::EntityType;

    assert(_data);
    auto& entityRomData = _data->entityRomData;

    ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
    if (ImGui::Begin(name)) {

        NamedListSidebar<ActionPolicy>(_data);

        SelectionT& sel = _data->*ActionPolicy::SelectionPtr;
        NamedList<EntityRomEntry>* list = ActionPolicy::getList(entityRomData);

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::BeginChild("Item");
        {
            if (list && sel.selectedIndex() < list->size()) {
                auto& entry = list->at(sel.selectedIndex());

                bool edited = false;

                ImGui::InputIdstring("Name", &entry.name);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                edited |= ImGui::IdStringCombo("Function Table", &entry.functionTable, entityRomData.functionTables);

                const auto functionTable = entityRomData.functionTables.find(entry.functionTable);

                ImGui::InputText("Comment", &entry.comment);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::Separator();

                edited |= ImGui::IdStringCombo("initialProjectileId", &entry.initialProjectileId, entityRomData.projectiles);

                if (ActionPolicy::entityType != EntityType::PLAYER) {
                    edited |= ImGui::IdStringCombo("initialListId", &entry.initialListId, entityRomData.listIds, false);
                }

                edited |= ImGui::IdStringCombo("frameSetId", &entry.frameSetId, projectFile.frameSets, false,
                                               [](auto& fsf) { return &fsf.name(); });

                if (ImGui::BeginCombo("displayFrame", entry.displayFrame)) {
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

                if (functionTable) {
                    const auto structChain = generateStructChain(functionTable->entityStruct);

                    ImGui::Separator();

                    unsigned id = 0;

                    for (auto it = structChain.rbegin(); it != structChain.rend(); it++) {
                        auto& st = entityRomData.structs.at(*it);

                        for (auto [i, field] : enumerate(st.fields)) {
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
                    ListActions<ActionPolicy>::selectedItemEdited(_data);
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndGroup();
    }
    ImGui::End();
}

void EntityRomDataEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    listIdsWindow();
    structsWindow();
    functionTablesWindow(projectFile);

    entityEntriesWindow<AP::Entities>("Entity ROM Entries", projectFile);

    entityEntriesWindow<AP::Projectiles>("Projectile ROM Entries", projectFile);

    entityEntriesWindow<AP::Players>("Player ROM Entries", projectFile);
}

// ::TODO replace with array_vector::
std::vector<unsigned> EntityRomDataEditorGui::generateStructChain(const idstring& name) const
{
    assert(_data);
    const auto& entityRomData = _data->entityRomData;

    std::vector<unsigned> items;
    items.reserve(8);

    unsigned sIndex = entityRomData.structs.indexOf(name);
    while (sIndex < entityRomData.structs.size()) {
        const bool containsParent = std::find(items.begin(), items.end(), sIndex) != items.end();
        if (containsParent) {
            items.clear();
            return items;
        }

        items.push_back(sIndex);

        const auto& st = entityRomData.structs.at(sIndex);
        sIndex = entityRomData.structs.indexOf(st.parent);
    }

    return items;
}

}
