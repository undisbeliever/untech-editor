/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "bytecode-editor.h"
#include "gui/aptable.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "models/common/iterators.h"
#include "models/scripting/scripting-error.h"

namespace UnTech::Gui {

// ActionPointsEditor Action Policies
struct BytecodeEditorData::AP {
    struct Bytecode {
        using EditorT = BytecodeEditorData;
        using EditorDataT = UnTech::Scripting::BytecodeInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.bytecode;
        }
    };

    struct Instructions : public Bytecode {
        using ListT = std::vector<Scripting::Instruction>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = 256 / 2;

        constexpr static auto SelectionPtr = &EditorT::instructionSel;

        static ListT* getList(EditorDataT& editorData)
        {
            return &editorData.instructions;
        }
    };
};

BytecodeEditorData::BytecodeEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
    , data()
    , instructionSel()
{
}

bool BytecodeEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    data = projectFile.bytecode;

    return true;
}

void BytecodeEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = Scripting::BytecodeErrorType;

    instructionSel.clearSelection();

    if (auto* e = dynamic_cast<const Scripting::BytecodeError*>(error)) {
        switch (e->type) {
        case Type::INSTRUCTION:
            instructionSel.setSelected(e->firstIndex);
            break;
        }
    }
}

void BytecodeEditorData::updateSelection()
{
    instructionSel.update();
}

BytecodeEditorGui::BytecodeEditorGui()
    : AbstractEditorGui("##Bytecode editor")
    , _data(nullptr)
{
}

bool BytecodeEditorGui::setEditorData(const std::shared_ptr<AbstractEditorData>& data)
{
    _data = std::dynamic_pointer_cast<BytecodeEditorData>(data);
    return _data != nullptr;
}

void BytecodeEditorGui::resetState()
{
}

void BytecodeEditorGui::editorClosed()
{
}

void BytecodeEditorGui::instructionsGui()
{
    assert(_data);
    auto& bytecode = _data->data;

    ImGui::TextUnformatted(u8"Bytecode Instructions:");

    ListButtons<AP::Instructions>(_data);

    constexpr auto columnNames = std::to_array({ "Name", "Argument 1", "Argument 2", "Yields" });

    if (beginApTable("Table", columnNames)) {
        for (auto& inst : bytecode.BASE_INSTRUCTIONS) {
            ImGui::TableNextColumn();

            ImGui::TableNextColumn();
            ImGui::TextUnformatted(inst.name);

            for (auto& a : inst.arguments) {
                ImGui::TableNextColumn();
                ImGui::TextEnum(a);
            }

            ImGui::TableNextColumn();
            bool yields = inst.yields;
            ImGui::Checkbox("Yields", &yields);
        }

        apTable_data<AP::Instructions>(
            _data,
            [&](auto& inst) { return Cell("##Name", &inst.name); },
            [&](auto& inst) { return Cell("##Arg1", &inst.arguments.at(0)); },
            [&](auto& inst) { return Cell("##Arg2", &inst.arguments.at(1)); },
            [&](auto& inst) { return Cell("Yields", &inst.yields); });

        endApTable();
    }
}

void BytecodeEditorGui::processGui(const Project::ProjectFile&, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    instructionsGui();
}

}
