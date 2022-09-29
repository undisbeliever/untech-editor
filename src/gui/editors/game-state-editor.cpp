/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "game-state-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "models/common/iterators.h"
#include "models/scripting/scripting-error.h"

namespace UnTech::Gui {

// GameStateEditor Action Policies
struct GameStateEditorData::AP {
    struct GameState {
        using EditorT = GameStateEditorData;
        using EditorDataT = UnTech::Scripting::GameState;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex&)
        {
            return &projectFile.gameState;
        }
    };

    struct Flags : public GameState {
        using ListT = std::vector<Scripting::GameStateFlag>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = Scripting::GameState::MAX_FLAGS;

        constexpr static auto SelectionPtr = &EditorT::flagSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.flags; }
    };

    struct Words : public GameState {
        using ListT = std::vector<Scripting::GameStateWord>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = Scripting::GameState::MAX_WORDS;

        constexpr static auto SelectionPtr = &EditorT::wordSel;

        static ListT* getList(EditorDataT& editorData) { return &editorData.words; }
    };
};

GameStateEditorData::GameStateEditorData(ItemIndex itemIndex)
    : AbstractEditorData(itemIndex)
    , data()
    , flagSel()
    , wordSel()
{
}

bool GameStateEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    data = projectFile.gameState;

    return true;
}

void GameStateEditorData::errorDoubleClicked(const AbstractError* error)
{
    using Type = Scripting::GameStateErrorType;

    flagSel.clearSelection();
    wordSel.clearSelection();

    if (auto* e = dynamic_cast<const Scripting::GameStateError*>(error)) {
        switch (e->type) {
        case Type::FLAG:
            flagSel.setSelected(e->firstIndex);
            break;

        case Type::WORD:
            wordSel.setSelected(e->firstIndex);
            break;
        }
    }
}

void GameStateEditorData::updateSelection()
{
    flagSel.update();
    wordSel.update();
}

GameStateEditorGui::GameStateEditorGui()
    : AbstractEditorGui()
    , _data(nullptr)
{
}

bool GameStateEditorGui::setEditorData(AbstractEditorData* data)
{
    return (_data = dynamic_cast<GameStateEditorData*>(data));
}

void GameStateEditorGui::resetState()
{
}

void GameStateEditorGui::editorClosed()
{
}

void GameStateEditorGui::gameStateWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& gameState = _data->data;

    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Game State")) {
        const auto& style = ImGui::GetStyle();

        const ImVec2 childSize = ImVec2((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2, 0);

        if (Cell("Starting Room", &gameState.startingRoom, projectFile.rooms)) {
            EditorActions<AP::GameState>::fieldEdited<
                &Scripting::GameState::startingRoom>(_data);
        }

        if (ImGui::BeginCombo("Starting Entrance", u8Cast(gameState.startingEntrance))) {
            if (const auto room = projectFile.rooms.find(gameState.startingRoom)) {
                const bool changed = ImGui::IdStringComboSelection(&gameState.startingEntrance, room->entrances, true);
                if (changed) {
                    EditorActions<AP::GameState>::fieldEdited<
                        &Scripting::GameState::startingEntrance>(_data);
                }
            }
            ImGui::EndCombo();
        }

        if (Cell("Starting Player", &gameState.startingPlayer, projectFile.entityRomData.players)) {
            EditorActions<AP::GameState>::fieldEdited<
                &Scripting::GameState::startingPlayer>(_data);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::BeginChild("FlagsChild", childSize, true);
        {
            ImGui::TextUnformatted(u8"Game State Flags:\n\n");

            apTable<AP::Flags>(
                "Flags", _data,
                std::to_array({ "Name", "Room" }),

                [&](auto& f) { return Cell("##name", &f.name); },
                [&](auto& f) { return Cell("##room", &f.room); });
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("WordsChild", childSize, true);
        {
            ImGui::TextUnformatted(u8"Game State Words:\n\n");

            apTable<AP::Words>(
                "Words", _data,
                std::to_array({ "Name", "Room", "Initial Value" }),

                [&](auto& w) { return Cell("##name", &w.name); },
                [&](auto& w) { return Cell("##room", &w.room); },
                [&](auto& w) { return Cell("##value", &w.initialValue); });
        }
        ImGui::EndChild();
    }

    ImGui::End();
}

void GameStateEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    gameStateWindow(projectFile);
}

}
