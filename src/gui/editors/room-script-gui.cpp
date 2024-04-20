/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "room-script-gui.h"
#include "room-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "gui/list-actions-variant.h"
#include "gui/style.h"
#include "models/project/project-data.h"

namespace UnTech::Gui {

// RoomScript Action Policies
struct RoomScriptGui::AP {
    struct RoomScripts {
        using EditorT = RoomEditorData;
        using EditorDataT = UnTech::Scripting::RoomScripts;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data.roomScripts;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            if (auto* room = fileListData(&projectFile.rooms, itemIndex.index)) {
                return &room->roomScripts;
            }
            else {
                return nullptr;
            }
        }
    };

    struct Scripts final : public RoomScripts {
        using ListT = NamedList<Scripting::Script>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Rooms::MAX_N_SCRIPTS;

        constexpr static auto SelectionPtr = &EditorT::scriptsSel;

        static ListT* getList(EditorDataT& roomScripts) { return &roomScripts.scripts; }
    };

    struct ScriptStatements final : public RoomScripts {
        using ListT = std::vector<Scripting::ScriptNode>;
        using ListArgsT = std::tuple<NodeSelection::ParentIndexT>;
        using SelectionT = NodeSelection;
        using ParentActionPolicy = RoomScriptGui::AP::Scripts;

        constexpr static size_t MAX_SIZE = NodeSelection::MAX_SIZE;

        constexpr static auto SelectionPtr = &EditorT::scriptStatementsSel;

        static ListT* getList(EditorDataT& roomScripts, const NodeSelection::ParentIndexT& parentIndex)
        {
            ListT* parent = nullptr;

            if (parentIndex.front() < roomScripts.scripts.size()) {
                parent = &roomScripts.scripts.at(parentIndex.front()).statements;
            }
            else {
                parent = &roomScripts.startupScript.statements;
            }

            // parent node index data format: eiii iiii iiii iiii (i = list index, e = else branch)
            for (const auto& node : skip_first_element(parentIndex)) {
                const unsigned i = node & 0x7fff;
                const bool elseFlag = node & 0x8000;

                if (i >= parent->size()) {
                    return parent;
                }
                Scripting::ScriptNode& statement = parent->at(i);

                if (std::get_if<UnTech::Scripting::Statement>(&statement)) {
                    return parent;
                }
                if (auto* s = std::get_if<UnTech::Scripting::IfStatement>(&statement)) {
                    parent = !elseFlag ? &s->thenStatements : &s->elseStatements;
                }
                if (auto* s = std::get_if<UnTech::Scripting::WhileStatement>(&statement)) {
                    parent = &s->statements;
                }
            }

            return parent;
        }
    };

    struct TempScriptFlags final : public RoomScripts {
        using ListT = std::vector<idstring>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = MultipleSelection::MAX_SIZE;

        constexpr static auto SelectionPtr = &EditorT::tempScriptFlagsSel;

        static ListT* getList(EditorDataT& roomScripts) { return &roomScripts.tempFlags; }
    };

    struct TempScriptWords final : public RoomScripts {
        using ListT = std::vector<idstring>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = MultipleSelection::MAX_SIZE;

        constexpr static auto SelectionPtr = &EditorT::tempScriptWordsSel;

        static ListT* getList(EditorDataT& roomScripts) { return &roomScripts.tempWords; }
    };
};

// cppcheck-suppress functionStatic
bool RoomScriptGui::roomArgument(const char* label, std::u8string* value, const Project::ProjectFile& pf) const
{
    bool edited = false;

    if (ImGui::BeginCombo(label, u8Cast(*value))) {
        for (const auto& item : pf.rooms) {
            if (item.value && item.value->name.isValid()) {
                const auto& roomName = item.value->name;
                if (ImGui::Selectable(u8Cast(roomName), roomName.str() == *value)) {
                    *value = roomName.str();
                    edited = true;
                }
            }
        }

        ImGui::EndCombo();
    }

    if (ImGui::IsItemHovered()) {
        ImGui::ShowTooltip(u8"Room");
    }

    return edited;
}

template <typename F>
static bool roomEntranceArgument(const char* label, std::u8string* value, F roomGetter)
{
    bool edited = false;

    if (ImGui::BeginCombo(label, u8Cast(*value))) {
        if (const auto r = roomGetter()) {
            for (const auto& en : r->entrances) {
                if (en.name.isValid()) {
                    if (ImGui::Selectable(u8Cast(en.name), en.name.str() == *value)) {
                        *value = en.name.str();
                        edited = true;
                    }
                }
            }
        }

        ImGui::EndCombo();
    }

    if (ImGui::IsItemHovered()) {
        ImGui::ShowTooltip(u8"Room Entrance");
    }

    return edited;
}

bool RoomScriptGui::roomAndRoomEntraceArguments(std::array<std::u8string, 2>& arguments, const Project::ProjectFile& pf)
{
    bool edited = false;

    ImGui::SameLine();
    edited |= roomArgument(argLabels.at(0), &arguments.at(0), pf);

    ImGui::SameLine();
    edited |= roomEntranceArgument(argLabels.at(1), &arguments.at(1),
                                   [&]() { return pf.rooms.find(idstring::fromString(arguments.at(0))); });

    return edited;
}

bool RoomScriptGui::statementArgument(const char* label, const Scripting::ArgumentType& type, std::u8string* value, const Project::ProjectFile& pf)
{
    using Type = Scripting::ArgumentType;

    bool edited = false;

    switch (type) {
    case Type::Unused: {
    } break;

    case Type::Flag:
    case Type::Word:
    case Type::ImmediateU16:
    case Type::RoomScript:
    case Type::EntityGroup: {
        ImGui::SameLine();
        edited = Cell(label, value);

        if (ImGui::IsItemHovered()) {
            switch (type) {
            case Type::Unused:
                break;

            case Type::Flag:
                ImGui::ShowTooltip(u8"Flag");
                break;

            case Type::Word:
                ImGui::ShowTooltip(u8"Word");
                break;

            case Type::ImmediateU16:
                ImGui::ShowTooltip(u8"Immediate U16");
                break;

            case Type::RoomScript:
                ImGui::ShowTooltip(u8"Room Script");
                break;

            case Type::EntityGroup:
                ImGui::ShowTooltip(u8"Entity Group");
                break;

            case Type::Room:
                ImGui::ShowTooltip(u8"Room");
                break;

            case Type::RoomEntrance:
                ImGui::ShowTooltip(u8"Room Entrance");
                break;
            }
        }
    } break;

    case Type::Room: {
        edited = roomArgument(label, value, pf);
    } break;

    case Type::RoomEntrance: {
        edited = roomEntranceArgument(label, value,
                                      [this]() { return &_data->data; });
    } break;
    }

    return edited;
}

bool RoomScriptGui::scriptArguments(const Scripting::InstructionData& bc, std::array<std::u8string, 2>& arguments, const Project::ProjectFile& pf)
{
    constexpr std::array<Scripting::ArgumentType, 2> loadRoomArgs = { Scripting::ArgumentType::Room, Scripting::ArgumentType::RoomEntrance };

    if (bc.arguments == loadRoomArgs) {
        return roomAndRoomEntraceArguments(arguments, pf);
    }
    else {
        bool edited = false;

        assert(bc.arguments.size() == arguments.size());
        for (const auto i : range(arguments.size())) {
            edited |= statementArgument(argLabels.at(i), bc.arguments.at(i), &arguments.at(i), pf);
        }

        return edited;
    }
}

bool RoomScriptGui::condition(Scripting::Conditional* c)
{
    bool edited = false;

    ImGui::SetNextItemWidth(75);
    edited |= Cell("##type", &c->type);
    if (edited) {
        switch (c->type) {
        case Scripting::ConditionalType::Flag:
            c->comparison = Scripting::ComparisonType::Set;
            c->value.clear();
            break;

        case Scripting::ConditionalType::Word:
            c->comparison = Scripting::ComparisonType::Equal;
            break;
        }
    }

    ImGui::SameLine();
    edited |= Cell("##var", &c->variable);

    ImGui::SameLine();
    ImGui::SetNextItemWidth(75);
    edited |= Cell("##comp", &c->comparison, c->type);

    switch (c->type) {
    case Scripting::ConditionalType::Flag:
        break;

    case Scripting::ConditionalType::Word:
        ImGui::SameLine();
        edited |= Cell("##value", &c->value);
        break;
    }

    return edited;
}

void RoomScriptGui::scriptNode_statement(Scripting::Statement& statement, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    bool edited = false;

    edited |= Cell("##Name", &statement.opcode, bcMapping.instructionNames);

    auto it = bcMapping.instructions.find(statement.opcode);
    if (it != bcMapping.instructions.end()) {
        const auto& bc = it->second;

        edited |= scriptArguments(bc, statement.arguments, pf);
    }

    if (edited) {
        ListActions<AP::ScriptStatements>::itemEdited(_data, parentIndex, index);
    }
}

void RoomScriptGui::scriptNode_ifStatement(Scripting::IfStatement& s, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    ImGui::TextUnformatted(u8"if ");
    ImGui::SameLine();

    const bool edited = condition(&s.condition);
    if (edited) {
        ListActionsVariant<AP::ScriptStatements>::variantFieldEdited<&Scripting::IfStatement::condition>(_data, parentIndex, index);
    }

    processChildStatements(s.thenStatements, s.elseStatements, pf, bcMapping);
}

void RoomScriptGui::scriptNode_whileStatement(Scripting::WhileStatement& s, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    ImGui::TextUnformatted(u8"while ");
    ImGui::SameLine();

    const bool edited = condition(&s.condition);
    if (edited) {
        ListActionsVariant<AP::ScriptStatements>::variantFieldEdited<&Scripting::WhileStatement::condition>(_data, parentIndex, index);
    }

    processChildStatements(s.statements, pf, bcMapping);
}

void RoomScriptGui::scriptNode_comment(Scripting::Comment& comment)
{
    ImGui::PushStyleColor(ImGuiCol_Text, Style::commentColor());

    bool edited = false;

    ImGui::TextUnformatted(u8"//");
    ImGui::SameLine();

    ImGui::SetNextItemWidth(-1);
    edited |= Cell("##Text", &comment.text);

    if (edited) {
        ListActions<AP::ScriptStatements>::itemEdited(_data, parentIndex, index);
    }

    ImGui::PopStyleColor();
}

void RoomScriptGui::scriptNode(Scripting::ScriptNode& node, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    // WHY!!!???
    //
    // No simple way to add reference arguments to std::visit.
    // This works for now, and the compiler will catch any typing mistakes.

    static_assert(std::variant_size_v<Scripting::ScriptNode> == 4);

    switch (node.index()) {
    case 0: {
        return scriptNode_statement(std::get<0>(node), pf, bcMapping);
    }
    case 1: {
        return scriptNode_ifStatement(std::get<1>(node), pf, bcMapping);
    }
    case 2: {
        return scriptNode_whileStatement(std::get<2>(node), pf, bcMapping);
    }
    case 3: {
        return scriptNode_comment(std::get<3>(node));
    }
    }
}

void RoomScriptGui::processStatements_afterParentIndexUpdated(std::vector<Scripting::ScriptNode>& statements, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    auto& sel = _data->scriptStatementsSel;

    const bool isParentSelected = parentIndex == sel.parentIndex();

    // Save index - prevents an infinite loop
    const unsigned oldIndex = index;

    const float selSpacing = ImGui::GetCursorPosX() + INDENT_SPACING;

    for (auto [i, s] : enumerate(statements)) {
        index = i;

        ImGui::PushID(index);

        ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);
        const std::u8string label = stringBuilder(index);
        if (ImGui::Selectable(u8Cast(label), isParentSelected && index == sel.selectedIndex(), ImGuiSelectableFlags_AllowOverlap)) {
            sel.setSelected(parentIndex, index);
        }
        ImGui::PopStyleColor();

        ImGui::SameLine(selSpacing);
        scriptNode(s, pf, bcMapping);

        ImGui::PopID();
    }

    // restore index
    index = oldIndex;
}

void RoomScriptGui::processStatements_root(std::vector<Scripting::ScriptNode>& statements, const unsigned scriptId, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    depth = 0;
    parentIndex.fill(0xffff);
    parentIndex.front() = scriptId;
    index = scriptId;

    processStatements_afterParentIndexUpdated(statements, pf, bcMapping);

    if (ImGui::Button("Add")) {
        openProcessMenu();
    }
}

void RoomScriptGui::processChildStatements(std::vector<Scripting::ScriptNode>& statements, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    if (depth + 1 >= parentIndex.size()) {
        ImGui::TextUnformatted(u8"ERROR: MAXIMUM DEPTH REACHED");
        return;
    }
    depth++;
    parentIndex.at(depth) = index & 0x7fff;

    ImGui::Indent(INDENT_SPACING);

    processStatements_afterParentIndexUpdated(statements, pf, bcMapping);

    if (ImGui::Button("Add")) {
        openProcessMenu();
    }

    ImGui::Unindent(INDENT_SPACING);

    parentIndex.at(depth) = NodeSelection::NO_SELECTION;
    depth--;
}

void RoomScriptGui::processChildStatements(std::vector<Scripting::ScriptNode>& thenStatements, std::vector<Scripting::ScriptNode>& elseStatements, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping)
{
    if (depth + 1 >= parentIndex.size()) {
        ImGui::TextUnformatted(u8"ERROR: MAXIMUM DEPTH REACHED");
        return;
    }
    depth++;
    parentIndex.at(depth) = index & 0x7fff;

    ImGui::Indent(INDENT_SPACING);

    processStatements_afterParentIndexUpdated(thenStatements, pf, bcMapping);

    if (ImGui::Button("Add")) {
        openProcessMenu();
    }

    // set else flag in parentIndex.
    parentIndex.at(depth) |= 0x8000;

    if (!elseStatements.empty()) {
        ImGui::TextUnformatted(u8"else");

        processStatements_afterParentIndexUpdated(elseStatements, pf, bcMapping);

        if (ImGui::Button("Add##Else")) {
            openProcessMenu();
        }
    }
    else {
        if (ImGui::Button("Add Else")) {
            openProcessMenu();
        }
    }

    ImGui::Unindent(INDENT_SPACING);

    parentIndex.at(depth) = NodeSelection::NO_SELECTION;
    depth--;
}

void RoomScriptGui::openProcessMenu()
{
    addMenuParentIndex = parentIndex;
    openAddMenu = true;
}

// ::TODO use menu to add statements anywhere::
void RoomScriptGui::processAddMenu(const Scripting::BytecodeMapping& bcMapping)
{
    using namespace UnTech::Scripting;

    if (openAddMenu) {
        ImGui::OpenPopup("Add To Script Menu");
    }

    if (ImGui::BeginPopup("Add To Script Menu")) {
        if (ImGui::BeginMenu("Statement")) {
            bool menuPressed = false;
            Scripting::Statement newStatement;

            for (auto& i : bcMapping.instructionNames) {
                if (ImGui::MenuItem(u8Cast(i))) {
                    newStatement.opcode = i;
                    menuPressed = true;
                }
            }

            if (menuPressed) {
                ListActions<AP::ScriptStatements>::addItem(_data, addMenuParentIndex, newStatement);
            }

            ImGui::EndMenu();
        }

        if (addMenuParentIndex.back() == UINT16_MAX) {
            // addMenuParentIndex is not at the maximum depth

            if (ImGui::BeginMenu("If")) {
                auto addIfStatement = [&](const ConditionalType type, const ComparisonType comp) {
                    IfStatement newStatement;
                    newStatement.condition.type = type;
                    newStatement.condition.comparison = comp;
                    ListActions<AP::ScriptStatements>::addItem(_data, addMenuParentIndex, newStatement);
                };

                if (ImGui::MenuItem("If flag set")) {
                    addIfStatement(ConditionalType::Flag, ComparisonType::Set);
                }
                if (ImGui::MenuItem("If flag clear")) {
                    addIfStatement(ConditionalType::Flag, ComparisonType::Clear);
                }
                if (ImGui::MenuItem("If word ==")) {
                    addIfStatement(ConditionalType::Word, ComparisonType::Equal);
                }
                if (ImGui::MenuItem("If word !=")) {
                    addIfStatement(ConditionalType::Word, ComparisonType::NotEqual);
                }
                if (ImGui::MenuItem("If word <")) {
                    addIfStatement(ConditionalType::Word, ComparisonType::LessThan);
                }
                if (ImGui::MenuItem("If word >=")) {
                    addIfStatement(ConditionalType::Word, ComparisonType::GreaterThanEqual);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("While")) {
                auto addWhileStatement = [&](const ConditionalType type, const ComparisonType comp) {
                    WhileStatement newStatement;
                    newStatement.condition.type = type;
                    newStatement.condition.comparison = comp;
                    ListActions<AP::ScriptStatements>::addItem(_data, addMenuParentIndex, newStatement);
                };

                if (ImGui::MenuItem("While flag set")) {
                    addWhileStatement(ConditionalType::Flag, ComparisonType::Set);
                }
                if (ImGui::MenuItem("While flag clear")) {
                    addWhileStatement(ConditionalType::Flag, ComparisonType::Clear);
                }
                if (ImGui::MenuItem("While word ==")) {
                    addWhileStatement(ConditionalType::Word, ComparisonType::Equal);
                }
                if (ImGui::MenuItem("While word !=")) {
                    addWhileStatement(ConditionalType::Word, ComparisonType::NotEqual);
                }
                if (ImGui::MenuItem("While word <")) {
                    addWhileStatement(ConditionalType::Word, ComparisonType::LessThan);
                }
                if (ImGui::MenuItem("While word >=")) {
                    addWhileStatement(ConditionalType::Word, ComparisonType::GreaterThanEqual);
                }

                ImGui::EndMenu();
            }
        }

        if (ImGui::MenuItem("Comment")) {
            ListActions<AP::ScriptStatements>::addItem(_data, addMenuParentIndex, Scripting::Comment{});
        }

        ImGui::EndPopup();
    }
}

template <typename AP>
static void tempVariableList(const char* strId, const std::shared_ptr<RoomEditorData>& data, const float outerHeight)
{
    apTable<AP>(
        strId, data,
        std::to_array({ "Name" }),
        ImVec2(0.0f, outerHeight),

        [&](auto& var) { return Cell("##name", &var); });
}

void RoomScriptGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData)
{
    assert(_data);
    auto& roomScripts = _data->data.roomScripts;

    ImGui::BeginChild("Temp-Vars", ImVec2(200, 0), true);
    {
        const float tableHeight = (ImGui::GetContentRegionAvail().y - 125) / 2;

        ImGui::TextUnformatted(u8"Temporary Flags:\n"
                               "(cleared on room load)");

        tempVariableList<AP::TempScriptFlags>("Flags", _data, tableHeight);

        ImGui::Spacing();
        ImGui::Spacing();

        ImGui::TextUnformatted(u8"Temporary Words:\n"
                               "(reset to 0 on room load)");

        tempVariableList<AP::TempScriptWords>("Words", _data, tableHeight);
    }
    ImGui::EndChild();
    ImGui::SameLine();

    ImGui::BeginChild("Sidebar", ImVec2(200, 0), true);
    {
        auto& sel = _data->scriptsSel;

        ListButtons<AP::Scripts>(_data);

        ImGui::BeginChild("struct-list");

        if (ImGui::Selectable("##Startup_Script", !sel.hasSelection(), ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick)) {
            sel.clearSelection();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(u8"Startup Script");

        for (auto [i, script] : enumerate(roomScripts.scripts)) {
            ImGui::PushID(i);

            ImGui::Selectable("##sel", &sel, i, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowDoubleClick);
            ImGui::SameLine();
            ImGui::TextUnformatted(script.name);

            ImGui::PopID();
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Script");

    const auto bcMapping = projectData.projectSettingsData.bytecodeData();
    if (bcMapping) {
        const bool isStartupScript = _data->scriptsSel.selectedIndex() >= roomScripts.scripts.size();
        const unsigned scriptId = !isStartupScript ? _data->scriptsSel.selectedIndex() : NodeSelection::NO_SELECTION;
        Scripting::Script& script = !isStartupScript ? roomScripts.scripts.at(scriptId) : roomScripts.startupScript;

        if (!isStartupScript) {
            if (Cell("Name", &script.name)) {
                ListActions<AP::Scripts>::selectedFieldEdited<
                    &Scripting::Script::name>(_data);
            }
        }
        else {
            ImGui::TextUnformatted(u8"Startup Script.\n"
                                   "This script will be started automatically on room load.\n"
                                   "The gameloop will not start until this script has finished execution.\n"
                                   "Start_Script will not activate a script until after the startup script ends.");
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ListButtons<AP::ScriptStatements>(_data);
        ImGui::Spacing();

        ImGui::BeginChild("Scroll");

        {
            ImGui::PushItemWidth((ImGui::GetWindowWidth() - 30) / 4);

            processStatements_root(script.statements, scriptId, projectFile, *bcMapping);

            ImGui::PopItemWidth();

            processAddMenu(*bcMapping);

            assert(depth == 0);
        }

        ImGui::EndChild();
    }
    else {
        ImGui::TextUnformatted(u8"\n\n"
                               "ERROR: Cannot view script: Missing bytecode data.");
    }

    ImGui::EndChild();
}

}
