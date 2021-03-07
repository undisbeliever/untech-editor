/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "game-state-editor.h"
#include "gui/editor-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"

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

void GameStateEditorGui::editorDataChanged()
{
}

void GameStateEditorGui::editorOpened()
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

        const ImVec2 childSize = ImVec2((ImGui::GetWindowContentRegionWidth() - style.ItemSpacing.x) / 2, 0);

        if (ImGui::IdStringCombo("Starting Room", &gameState.startingRoom, projectFile.rooms)) {
            EditorActions<AP::GameState>::fieldEdited<
                &Scripting::GameState::startingRoom>(_data);
        }

        if (ImGui::BeginCombo("Starting Entrance", gameState.startingEntrance)) {
            if (const auto* room = projectFile.rooms.find(gameState.startingRoom)) {
                const bool changed = ImGui::IdStringComboSelection(&gameState.startingEntrance, room->entrances, true);
                if (changed) {
                    EditorActions<AP::GameState>::fieldEdited<
                        &Scripting::GameState::startingEntrance>(_data);
                }
            }
            ImGui::EndCombo();
        }

        if (ImGui::IdStringCombo("Starting Player", &gameState.startingPlayer, projectFile.entityRomData.players)) {
            EditorActions<AP::GameState>::fieldEdited<
                &Scripting::GameState::startingPlayer>(_data);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::BeginChild("FlagsChild", childSize, true);
        {
            ImGui::TextUnformatted("Game State Flags:\n\n");
            ListButtons<AP::Flags>(_data);

            ImGui::BeginChild("Scroll");

            ImGui::Columns(3);
            ImGui::SetColumnWidth(0, 40);

            ImGui::Separator();
            ImGui::NextColumn();
            ImGui::Text("Name");
            ImGui::NextColumn();
            ImGui::Text("Room");
            ImGui::NextColumn();
            ImGui::Separator();

            for (unsigned i = 0; i < gameState.flags.size(); i++) {
                auto& f = gameState.flags.at(i);

                bool edited = false;

                ImGui::PushID(i);

                ImGui::Selectable(&_data->flagSel, i);
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1);
                ImGui::InputIdstring("##Name", &f.name);
                edited |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1);
                edited |= ImGui::IdStringCombo("##Room", &f.room, projectFile.rooms, true);
                ImGui::NextColumn();

                if (edited) {
                    ListActions<AP::Flags>::itemEdited(_data, i);
                }

                ImGui::PopID();
            }

            ImGui::Columns(1);
            ImGui::Separator();

            ImGui::EndChild();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("WordsChild", childSize, true);
        {
            ImGui::TextUnformatted("Game State Words:\n\n");
            ListButtons<AP::Words>(_data);

            ImGui::BeginChild("Scroll");

            ImGui::Columns(4);
            ImGui::SetColumnWidth(0, 40);

            ImGui::Separator();
            ImGui::NextColumn();
            ImGui::Text("Name");
            ImGui::NextColumn();
            ImGui::Text("Room");
            ImGui::NextColumn();
            ImGui::Text("Initial Value");
            ImGui::NextColumn();
            ImGui::Separator();

            for (unsigned i = 0; i < gameState.words.size(); i++) {
                auto& w = gameState.words.at(i);

                bool edited = false;

                ImGui::PushID(i);

                ImGui::Selectable(&_data->wordSel, i);
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1);
                ImGui::InputIdstring("##Name", &w.name);
                edited |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1);
                edited |= ImGui::IdStringCombo("##Room", &w.room, projectFile.rooms, true);
                ImGui::NextColumn();

                ImGui::SetNextItemWidth(-1);
                ImGui::InputUint16("##Value", &w.initialValue);
                edited |= ImGui::IsItemDeactivatedAfterEdit();
                ImGui::NextColumn();

                if (edited) {
                    ListActions<AP::Words>::itemEdited(_data, i);
                }

                ImGui::PopID();
            }

            ImGui::Columns(1);
            ImGui::Separator();

            ImGui::EndChild();
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
