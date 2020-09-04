/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "room-editor.h"
#include "gui/common/aabb-graphics.h"
#include "gui/common/entity-graphics.h"
#include "gui/editor-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "models/rooms/rooms-serializer.h"

namespace UnTech::Gui {

namespace RM = UnTech::Rooms;

// ::TODO move to style::
constexpr static ImU32 entityOutlineCol = IM_COL32(192, 0, 0, 240);
constexpr static ImU32 entityFillCol = IM_COL32(192, 0, 0, 64);
constexpr static ImU32 entranceOutlineCol = IM_COL32(0, 192, 0, 240);
constexpr static ImU32 entranceFillCol = IM_COL32(0, 192, 0, 64);

constexpr static ImU32 disabledEntityGroupTint = IM_COL32(255, 255, 255, 32);

bool RoomEditor::_tilesetAndPaletteIndexValid = false;
AabbGraphics RoomEditor::_graphics;

std::shared_ptr<const EntityGraphics> RoomEditor::_entityGraphics = nullptr;

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;

// RoomEditor Action Policies
struct RoomEditor::AP {
    struct Room {
        using EditorT = RoomEditor;
        using EditorDataT = UnTech::Rooms::RoomInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor._data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return fileListData(&projectFile.rooms, itemIndex.index);
        }
    };

    struct Entrances final : public Room {
        using ListT = NamedList<UnTech::Rooms::RoomEntrance>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = RM::MAX_ROOM_ENTRANCES;

        constexpr static auto SelectionPtr = &EditorT::_entrancesSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.entrances; }
    };

    struct EntityGroups final : public Room {
        using ListT = NamedList<UnTech::Rooms::EntityGroup>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = RM::MAX_ENTITY_GROUPS;

        constexpr static auto SelectionPtr = &EditorT::_entityGroupsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.entityGroups; }
    };

    struct EntityEntries final : public Room {
        using ListT = std::vector<UnTech::Rooms::EntityEntry>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = GroupMultipleSelection;
        using ParentActionPolicy = RoomEditor::AP::EntityGroups;

        // ::TODO increase to RM::MAX_ENTITY_ENTRIES::
        // ::: Or maybe make a maximum of 64 entites per group::
        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::_entityEntriesSel;

        static ListT* getList(EditorDataT& entityRomData, unsigned groupIndex)
        {
            return getListField(EntityGroups::getList(entityRomData), groupIndex,
                                &RM::EntityGroup::entities);
        }
    };
};

Texture& RoomEditor::entityTexture()
{
    static Texture texture;
    return texture;
}

RoomEditor::RoomEditor(ItemIndex itemIndex)
    : AbstractMetaTileEditor(itemIndex)
{
}

bool RoomEditor::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    // This function is called before undo/redo. Invalidate data so it can recalulated next frame.
    AbstractMetaTileEditor::resetState();

    _tilesetAndPaletteIndexValid = false;

    const auto [data, fn] = fileListItem(&projectFile.rooms, itemIndex().index);
    setFilename(fn);
    if (data) {
        _data = *data;
    }
    return data != nullptr;
}

void RoomEditor::saveFile() const
{
    assert(!filename().empty());
    UnTech::Rooms::saveRoomInput(_data, filename());
}

void RoomEditor::editorOpened()
{
    AbstractMetaTileEditor::editorOpened();

    setEditMode(EditMode::SelectObjects);

    _mapSize = _data.map.size();
}

void RoomEditor::editorClosed()
{
    // ::TODO find a way to save scrollbar positions and restore them when the editor is opened again::
}

grid<uint8_t>& RoomEditor::map()
{
    return _data.map;
}

void RoomEditor::mapTilesPlaced(const urect r)
{
    assert(_data.map.size().contains(r));

    // ::TODO add grid editor action::
    EditorActions<AP::Room>::fieldEdited<
        &RM::RoomInput::map>(this);
}

void RoomEditor::selectedTilesetTilesChanged()
{
    _selectedTiles.clear();
    _selectedScratchpadTiles.clear();
}

void RoomEditor::selectedTilesChanged()
{
    _selectedTilesetTiles.clear();
    _selectedScratchpadTiles.clear();
}

void RoomEditor::selectedScratchpadTilesChanged()
{
    _selectedTiles.clear();
    _selectedTilesetTiles.clear();
}

void RoomEditor::clearSelectedTiles()
{
    _selectedTilesetTiles.clear();
    _selectedTiles.clear();
    _selectedScratchpadTiles.clear();
}

void RoomEditor::propertiesWindow(const Project::ProjectFile& projectFile)
{
    const static usize minMapSize(RM::RoomInput::MIN_MAP_WIDTH, RM::RoomInput::MIN_MAP_HEIGHT);
    const static usize maxMapSize(RM::RoomInput::MAX_MAP_WIDTH, RM::RoomInput::MAX_MAP_HEIGHT);

    if (ImGui::Begin("Room Properties")) {
        ImGui::SetWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

        ImGui::InputIdstring("Name", &_data.name);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::name>(this);
        }

        if (ImGui::IdStringCombo("Scene", &_data.scene, projectFile.resourceScenes.scenes)) {
            _tilesetAndPaletteIndexValid = false;

            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::scene>(this);
        }

        if (ImGui::InputUsize("Map Size", &_mapSize, maxMapSize)) {
            _mapSize.width = std::max(_mapSize.width, minMapSize.width);
            _mapSize.height = std::max(_mapSize.height, minMapSize.height);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            // ::TODO use a GridAction to resize scratchpad::
            _data.map = _data.map.resized(_mapSize, 0);
            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::map>(this);
        }
        if (!ImGui::IsItemActive()) {
            // ::TODO use callback to update scratchpad size::
            _mapSize = _data.map.size();
        }
    }

    ImGui::End();
}

void RoomEditor::entrancesWindow()
{
    if (ImGui::Begin("Entrances##Room", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::SetWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);

        ListButtons<AP::Entrances>(this);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(4);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Name");
        ImGui::NextColumn();
        ImGui::Text("Position");
        ImGui::NextColumn();
        ImGui::Text("Orientation");
        ImGui::NextColumn();
        ImGui::Separator();

        const usize bounds(_data.mapRight(), _data.mapBottom());

        for (unsigned i = 0; i < _data.entrances.size(); i++) {
            auto& en = _data.entrances.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_entrancesSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputIdstring("##name", &en.name);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputUpoint("##pos", &en.position, bounds);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::EnumCombo("##orientation", &en.orientation);
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::Entrances>::itemEdited(this, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void RoomEditor::roomEntitiesWindow(const Project::ProjectFile& projectFile)
{
    using namespace std::string_literals;

    constexpr static std::array<const char*, RM::MAX_ENTITY_GROUPS + 4> entityGroupNames{
        "Entity Group 0",
        "Entity Group 1",
        "Entity Group 2",
        "Entity Group 3",
        "Entity Group 4",
        "Entity Group 5",
        "Entity Group 6",
        "Entity Group 7",
        "Entity Group OUT OF BOUNDS",
        "Entity Group OUT OF BOUNDS",
        "Entity Group OUT OF BOUNDS",
        "Entity Group OUT OF BOUNDS",
    };

    if (ImGui::Begin("Entities##Room_Entities", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::SetWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);

        const rect bounds = _data.validEntityArea();

        ImGui::PushID("Entity Groups");
        ListButtons<AP::EntityGroups>(this);
        ImGui::PopID();
        ImGui::PushID("Entity Entries");
        ListButtons<AP::EntityEntries>(this);
        ImGui::PopID();

        ImGui::BeginChild("Scroll");

        // ::TODO scroll to selected entity/group when selection changes::

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

        unsigned entityId = 0;

        const unsigned nGroups = std::min(_data.entityGroups.size(), entityGroupNames.size());
        for (unsigned groupIndex = 0; groupIndex < nGroups; groupIndex++) {
            auto& group = _data.entityGroups.at(groupIndex);

            if (ImGui::TreeNodeToggleSelection(entityGroupNames.at(groupIndex), &_entityGroupsSel, groupIndex)) {
                {
                    bool edited = false;

                    ImGui::InputIdstring("Name", &group.name);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    if (edited) {
                        ListActions<AP::EntityGroups>::itemEdited(this, groupIndex);
                    }
                }
                ImGui::Spacing();

                for (unsigned i = 0; i < group.entities.size(); i++) {
                    auto& entity = group.entities.at(i);

                    bool edited = false;

                    ImGui::PushID(i);

                    const std::string selLabel = "Entity "s + std::to_string(entityId);
                    entityId++;

                    ImGui::Selectable(selLabel.c_str(), &_entityEntriesSel, groupIndex, i);

                    ImGui::Indent();

                    ImGui::InputIdstring("Name", &entity.name);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    edited |= ImGui::IdStringCombo("Entity Id", &entity.entityId, projectFile.entityRomData.entities);

                    ImGui::InputPoint("Position", &entity.position, bounds);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    ImGui::InputText("Parameter", &entity.parameter);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    if (edited) {
                        ListActions<AP::EntityEntries>::itemEdited(this, groupIndex, i);
                    }

                    ImGui::Unindent();
                    ImGui::PopID();
                }
                ImGui::Spacing();

                ImGui::TreePop();
            }
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void RoomEditor::drawObjects(ImDrawList* drawList) const
{
    assert(_entityGraphics);

    // ::TODO dynamic playerIds::
    constexpr unsigned playerId = 0;

    const ImTextureID textureId = entityTexture().imguiTextureId();

    // NOTE: pushing a textureId to the drawlist prevents me from drawing filled rects
    drawList->PushTextureID(textureId);

    // ::TODO show/hide entrances::
    if (true) {
        for (const auto& entrance : _data.entrances) {
            _graphics.drawEntity(drawList, &entrance.position,
                                 textureId, _entityGraphics->settingsForPlayer(playerId), IM_COL32_WHITE);
        }
    }

    // ::TODO show/hide entities::
    if (true) {
        const unsigned nGroups = std::min<size_t>(_entityEntriesSel.MAX_GROUP_SIZE, _data.entityGroups.size());
        for (unsigned groupIndex = 0; groupIndex < nGroups; groupIndex++) {
            const auto& group = _data.entityGroups.at(groupIndex);

            const bool groupEnabled = _entityGroupsSel.selectedIndex() > nGroups || _entityGroupsSel.selectedIndex() == groupIndex;

            const ImU32 tint = groupEnabled ? IM_COL32_WHITE : disabledEntityGroupTint;

            for (const auto& entity : group.entities) {
                _graphics.drawEntity(drawList, &entity.position,
                                     textureId, _entityGraphics->settingsForEntity(entity.entityId),
                                     tint);
            }
        }
    }
}

void RoomEditor::drawAndEditObjects(ImDrawList* drawList)
{
    assert(_entityGraphics != nullptr);

    // ::TODO dynamic playerIds::
    constexpr unsigned playerId = 0;

    const ImTextureID textureId = entityTexture().imguiTextureId();
    // NOTE: Cannot push textureId to drawList - it prevents me from drawing filled rects.

    // ::TODO show/hide entrances::
    if (true) {
        for (unsigned i = 0; i < _data.entrances.size(); i++) {
            auto& entrance = _data.entrances.at(i);

            _graphics.addEntity(drawList, &entrance.position,
                                textureId, _entityGraphics->settingsForPlayer(playerId),
                                entranceFillCol, entranceOutlineCol, IM_COL32_WHITE,
                                &_entrancesSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                ImGui::Text("Extrance %u %s", i, entrance.name.str().c_str());
                ImGui::EndTooltip();
            }
        }
    }

    // ::TODO show/hide entities::
    if (true) {
        const unsigned nGroups = std::min<size_t>(_entityEntriesSel.MAX_GROUP_SIZE, _data.entityGroups.size());
        for (unsigned groupIndex = 0; groupIndex < nGroups; groupIndex++) {
            auto& group = _data.entityGroups.at(groupIndex);
            auto& childSel = _entityEntriesSel.childSel(groupIndex);

            if (_entityGroupsSel.selectedIndex() > nGroups || _entityGroupsSel.selectedIndex() == groupIndex) {
                // No entity groups are selected or the  groupIndex is the selected group

                for (unsigned i = 0; i < group.entities.size(); i++) {
                    auto& entity = group.entities.at(i);

                    _graphics.addEntity(drawList, &entity.position,
                                        textureId, _entityGraphics->settingsForEntity(entity.entityId),
                                        entityFillCol, entityOutlineCol, IM_COL32_WHITE,
                                        &childSel, i);
                    if (_graphics.isHoveredAndNotEditing()) {
                        ImGui::BeginTooltip();
                        ImGui::Text("Entity %u (%s)", i, entity.entityId.str().c_str());
                        ImGui::Indent();
                        if (entity.name.isValid()) {
                            ImGui::Text("Name: %s", entity.name.str().c_str());
                        }
                        ImGui::Text("Group: %u (%s)", groupIndex, group.name.str().c_str());
                        if (!entity.parameter.empty()) {
                            ImGui::Text("Parameter: %s", entity.parameter.c_str());
                        }
                        ImGui::Unindent();
                        ImGui::EndTooltip();
                    }
                }
            }
            else {
                // A group is selected and groupIndex is not the selected group

                for (const auto& entity : group.entities) {
                    _graphics.drawEntity(drawList, &entity.position,
                                         textureId, _entityGraphics->settingsForEntity(entity.entityId),
                                         disabledEntityGroupTint);
                }
            }
        }
    }
}

void RoomEditor::editorWindow()
{
    // ::TODO shrink entity vertical spacing at the top of the map::

    if (ImGui::Begin("Room###Room_Editor")) {
        ImGui::SetWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);

        // ::TODO expand toolbar::
        ImGui::TextUnformatted("::TODO expand toolBar::");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(180);
        ImGui::SingleSelectionNamedListCombo("##EntityGroupCombo", &_entityGroupsSel, _data.entityGroups, true);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Selected Entity Group");
            ImGui::EndTooltip();
        }

        if (selectObjectsButton()) {
            clearSelectedTiles();
        }
        ImGui::SameLine();
        editModeButtons();

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        // ::TODO zoom::
        const ImVec2 zoom(2.0f, 2.0f);
        const rect bounds = _data.validEntityArea();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        _graphics.setDisabled(editMode() != EditMode::SelectObjects);

        _graphics.startLoop("Room", bounds, zoom,
                            &_entrancesSel, &_entityEntriesSel);

        // ::TODO set initial scroll to position top-left of map ::

        const Geometry geo{
            ImVec2(METATILE_SIZE_PX * zoom.x, METATILE_SIZE_PX * zoom.y),
            ImVec2(_data.map.width() * METATILE_SIZE_PX * zoom.x, _data.map.height() * METATILE_SIZE_PX * zoom.y),
            _graphics.toVec2(0, 0),
            zoom
        };
        drawAndEditMap(geo);

        if (editMode() == EditMode::SelectObjects) {
            drawAndEditObjects(drawList);
        }
        else {
            drawObjects(drawList);
        }

        _graphics.endLoop(drawList,
                          &_entrancesSel, &_entityEntriesSel);

        if (_graphics.isEditingFinished()) {
            // ::TODO add action macros::
            ListActions<AP::Entrances>::selectedItemsEdited(this);
            ListActions<AP::EntityEntries>::selectedItemsEdited(this);
        }

        ImGui::EndChild();
    }
    ImGui::End();
}

void RoomEditor::processGui(const Project::ProjectFile& projectFile)
{
    updateEntityGraphics();
    updateTilesetAndPaletteIndex(projectFile);
    updateTextures(projectFile);

    propertiesWindow(projectFile);
    entrancesWindow();
    roomEntitiesWindow(projectFile);

    editorWindow();

    minimapWindow("Minimap##Room");
    tilesetMinimapWindow("Tileset##Room");

    if (scratchpadMinimapWindow("Scratchpad##Room", &_selectedScratchpadTiles, projectFile)) {
        selectedScratchpadTilesChanged();
    }
}

void RoomEditor::updateSelection()
{
    AbstractMetaTileEditor::updateSelection();

    _entrancesSel.update();
    _entityGroupsSel.update();

    if (_entityGroupsSel.hasSelection()) {
        // Disable entity selection if the group is not selected.
        for (unsigned groupIndex = 0; groupIndex < _entityEntriesSel.MAX_GROUP_SIZE; groupIndex++) {
            if (groupIndex != _entityGroupsSel.selectedIndex()) {
                _entityEntriesSel.childSel(groupIndex).clearSelection();
            }
        }
    }

    _entityEntriesSel.update();
}

void RoomEditor::updateEntityGraphics()
{
    // ::TODO retrieve _entityGraphics from ProjectData::
    if (_entityGraphics != nullptr) {
        return;
    }

    // ::TODO populate EntityGraphics data::
    _entityGraphics = std::make_shared<EntityGraphics>();

    entityTexture().replace(_entityGraphics->image());
}

void RoomEditor::updateTilesetAndPaletteIndex(const Project::ProjectFile& projectFile)
{
    // ::TODO invalidate tilesetIndex when ResourceScenes is compiled::

    if (_tilesetAndPaletteIndexValid) {
        return;
    }

    unsigned tilesetIndex = INT_MAX;
    unsigned paletteIndex = INT_MAX;

    // ::TODO use compiled data to get MetaTile id and palette id::

    if (auto scene = projectFile.resourceScenes.scenes.find(_data.scene)) {
        paletteIndex = projectFile.palettes.indexOf(scene->palette);

        if (auto sceneSettings = projectFile.resourceScenes.settings.find(scene->sceneSettings)) {
            for (unsigned layerId = 0; layerId < sceneSettings->layerTypes.size(); layerId++) {
                if (sceneSettings->layerTypes.at(layerId) == UnTech::Resources::LayerType::MetaTileTileset) {
                    const auto& mt = scene->layers.at(layerId);
                    if (mt.isValid()) {
                        tilesetIndex = projectFile.metaTileTilesets.indexOf(mt);
                        break;
                    }
                }
            }
        }
    }

    setTilesetIndex(tilesetIndex);
    setPaletteIndex(paletteIndex);

    _tilesetAndPaletteIndexValid = true;
}

}
