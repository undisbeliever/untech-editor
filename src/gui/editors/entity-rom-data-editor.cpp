/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "entity-rom-data-editor.h"
#include "gui/imgui.h"
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

EntityRomDataEditor::EntityRomDataEditor(ItemIndex itemIndex)
    : AbstractEditor(itemIndex)
{
}

bool EntityRomDataEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    _entityRomData = projectFile.entityRomData;

    return true;
}

void EntityRomDataEditor::commitPendingChanges(Project::ProjectFile& projectFile)
{
    projectFile.entityRomData = _entityRomData;
}

void EntityRomDataEditor::editorOpened()
{
}

void EntityRomDataEditor::editorClosed()
{
}

void EntityRomDataEditor::listIdsGui()
{
    if (ImGui::Begin("EntityListIds")) {
        ImGui::SetWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

        ListButtons(&_listIdsSel, &_entityRomData.listIds, 32);

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
                ImGui::LogText("Edited Interactive Tiles");
                this->pendingChanges = true;
            }

            ImGui::PopID();

            UpdateSelection(&_listIdsSel);
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

void EntityRomDataEditor::structsGui()
{
    if (ImGui::Begin("Entity Rom Structs")) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        NamedListSidebar(_entityRomData.structs, &_structsSel, 1024);

        UpdateChildSelection(&_structsSel, &_structFieldsSel);

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::BeginChild("Item");
        {
            if (_structsSel.selected < _entityRomData.structs.size()) {
                auto& st = _entityRomData.structs.at(_structsSel.selected);

                {
                    bool edited = false;

                    ImGui::InputIdstring("Name", &st.name);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    edited |= parentCombo("Parent", &st.parent, _entityRomData.structs, _structsSel.selected);

                    ImGui::InputText("Comment", &st.comment);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    if (edited) {
                        ImGui::LogText("Edited Entity ROM Struct");
                        this->pendingChanges = true;
                    }
                }

                ImGui::Separator();

                {
                    ListButtons(&_structFieldsSel, &st.fields, 64);

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
                            ImGui::LogText("Edited Entity ROM Struct Field");
                            this->pendingChanges = true;
                        }

                        ImGui::PopID();

                        UpdateSelection(&_structFieldsSel);
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

void EntityRomDataEditor::functionTablesGui(const UnTech::Project::ProjectFile& projectFile)
{
    if (ImGui::Begin("Entity Function Tables")) {
        ImGui::SetWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);

        ListButtons(&_functionTablesSel, &_entityRomData.functionTables, MetaSprite::MAX_ACTION_POINT_FUNCTIONS);

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
                ImGui::LogText("Edited Entity Function Table");
                this->pendingChanges = true;
            }

            ImGui::PopID();

            UpdateSelection(&_functionTablesSel);
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void EntityRomDataEditor::entriesGui(const char* name, EntityType type,
                                     NamedList<EntityRomEntry>& entries, SingleSelection* sel,
                                     const Project::ProjectFile& projectFile)
{
    if (ImGui::Begin(name)) {
        ImGui::SetWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);

        NamedListSidebar(entries, sel, UnTech::Entity::MAX_N_ENTITY_ENTRIES);

        ImGui::SameLine();

        ImGui::BeginGroup();
        ImGui::BeginChild("Item");
        {
            if (sel->selected < entries.size()) {
                auto& entry = entries.at(sel->selected);

                bool edited = false;

                ImGui::InputIdstring("Name", &entry.name);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                edited |= ImGui::IdStringCombo("Function Table", &entry.functionTable, _entityRomData.functionTables);

                const auto functionTable = _entityRomData.functionTables.find(entry.functionTable);

                ImGui::InputText("Comment", &entry.comment);
                edited |= ImGui::IsItemDeactivatedAfterEdit();

                ImGui::Separator();

                edited |= ImGui::IdStringCombo("initialProjectileId", &entry.initialListId, _entityRomData.projectiles);

                if (type != EntityType::PLAYER) {
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
                    ImGui::LogText("Edited Entity ROM Struct");
                    this->pendingChanges = true;
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
    listIdsGui();
    structsGui();
    functionTablesGui(projectFile);
    entriesGui("Entity ROM Entries", EntityType::ENTITY, _entityRomData.entities, &_entitiesSel, projectFile);
    entriesGui("Projectile ROM Entries", EntityType::PROJECTILE, _entityRomData.projectiles, &_projectilesSel, projectFile);
    entriesGui("Player ROM Entries", EntityType::PLAYER, _entityRomData.players, &_playersSel, projectFile);
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
