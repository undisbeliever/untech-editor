/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/imgui.h"
#include "models/project/project.h"
#include <array>
#include <memory>

namespace UnTech::Gui {

class RoomEditorData;

class RoomScriptGui {
    struct AP;

private:
    constexpr static float INDENT_SPACING = 30;

private:
    std::shared_ptr<RoomEditorData> _data;

    unsigned depth{ 0 };

    std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> parentIndex{};
    unsigned index{ 0 };

    constexpr static std::array<const char*, 2> argLabels = { "##Arg1", "##Arg2" };

    bool openAddMenu{ false };
    std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> addMenuParentIndex{};

    const ImVec4 disabledColor;

public:
    explicit RoomScriptGui()
        : _data(nullptr)
        , disabledColor(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled))
    {
    }

    void setEditorData(std::shared_ptr<RoomEditorData> data)
    {
        _data = std::move(data);
    }

    void processGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData);

private:
    bool roomArgument(const char* label, std::u8string* value, const Project::ProjectFile& pf) const;
    bool roomAndRoomEntraceArguments(std::array<std::u8string, 2>& arguments, const Project::ProjectFile& pf);
    bool statementArgument(const char* label, const Scripting::ArgumentType& type, std::u8string* value, const Project::ProjectFile& pf);
    bool scriptArguments(const Scripting::InstructionData& bc, std::array<std::u8string, 2>& arguments, const Project::ProjectFile& pf);

    bool condition(Scripting::Conditional* c);

    void scriptNode(Scripting::ScriptNode& node, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);
    void scriptNode_statement(Scripting::Statement& statement, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);
    void scriptNode_ifStatement(Scripting::IfStatement& s, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);
    void scriptNode_whileStatement(Scripting::WhileStatement& s, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);
    void scriptNode_comment(Scripting::Comment& comment);

    void processStatements_root(std::vector<Scripting::ScriptNode>& statements, const unsigned scriptId, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);
    void processStatements_afterParentIndexUpdated(std::vector<Scripting::ScriptNode>& statements, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);
    void processChildStatements(std::vector<Scripting::ScriptNode>& statements, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);
    void processChildStatements(std::vector<Scripting::ScriptNode>& thenStatements, std::vector<Scripting::ScriptNode>& elseStatements, const Project::ProjectFile& pf, const Scripting::BytecodeMapping& bcMapping);

    void openProcessMenu();
    void processAddMenu(const Scripting::BytecodeMapping& bcMapping);
};

}
