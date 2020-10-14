/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "room-editor.h"
#include "gui/editor-actions.h"
#include "gui/graphics/aabb-graphics.h"
#include "gui/graphics/entity-graphics.h"
#include "gui/grid-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions.h"
#include "gui/style.h"
#include "models/rooms/rooms-serializer.h"

namespace UnTech::Gui {

namespace RM = UnTech::Rooms;

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;

// RoomEditor Action Policies
struct RoomEditorData::AP {
    struct Room {
        using EditorT = RoomEditorData;
        using EditorDataT = UnTech::Rooms::RoomInput;

        static EditorDataT* getEditorData(EditorT& editor)
        {
            return &editor.data;
        }

        static EditorDataT* getEditorData(Project::ProjectFile& projectFile, const ItemIndex& itemIndex)
        {
            return fileListData(&projectFile.rooms, itemIndex.index);
        }
    };

    struct Map : public Room {
        using GridT = grid<uint8_t>;
        using ListArgsT = std::tuple<>;

        constexpr static unsigned MAX_WIDTH = RM::RoomInput::MAX_MAP_WIDTH;
        constexpr static unsigned MAX_HEIGHT = RM::RoomInput::MAX_MAP_HEIGHT;
        constexpr static uint8_t DEFAULT_VALUE = 0;

        constexpr static auto SelectionPtr = &EditorT::selectedTiles;

        static GridT* getGrid(EditorDataT& entityRomData) { return &entityRomData.map; }
    };

    struct Entrances final : public Room {
        using ListT = NamedList<UnTech::Rooms::RoomEntrance>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = RM::MAX_ROOM_ENTRANCES;

        constexpr static auto SelectionPtr = &EditorT::entrancesSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.entrances; }
    };

    struct EntityGroups final : public Room {
        using ListT = NamedList<UnTech::Rooms::EntityGroup>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = RM::MAX_ENTITY_GROUPS;

        constexpr static auto SelectionPtr = &EditorT::entityGroupsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.entityGroups; }
    };

    struct EntityEntries final : public Room {
        using ListT = std::vector<UnTech::Rooms::EntityEntry>;
        using ListArgsT = std::tuple<unsigned>;
        using SelectionT = GroupMultipleSelection;
        using ParentActionPolicy = RoomEditorData::AP::EntityGroups;

        // ::TODO increase to RM::MAX_ENTITY_ENTRIES::
        // ::: Or maybe make a maximum of 64 entites per group::
        constexpr static size_t MAX_SIZE = 64;

        constexpr static auto SelectionPtr = &EditorT::entityEntriesSel;

        static ListT* getList(EditorDataT& entityRomData, unsigned groupIndex)
        {
            return getListField(EntityGroups::getList(entityRomData), groupIndex,
                                &RM::EntityGroup::entities);
        }
    };
};

RoomEditorData::RoomEditorData(ItemIndex itemIndex)
    : AbstractMetaTileEditorData(itemIndex)
{
}

bool RoomEditorData::loadDataFromProject(const Project::ProjectFile& projectFile)
{
    // This function is called before undo/redo. Invalidate data so it can recalulated next frame.

    const auto [ptr, fn] = fileListItem(&projectFile.rooms, itemIndex().index);
    setFilename(fn);
    if (ptr) {
        data = *ptr;
    }
    return ptr != nullptr;
}

void RoomEditorData::saveFile() const
{
    assert(!filename().empty());
    UnTech::Rooms::saveRoomInput(data, filename());
}

void RoomEditorData::updateSelection()
{
    AbstractMetaTileEditorData::updateSelection();

    entrancesSel.update();
    entityGroupsSel.update();

    if (entityGroupsSel.hasSelection()) {
        // Disable entity selection if the group is not selected.
        for (unsigned groupIndex = 0; groupIndex < entityEntriesSel.MAX_GROUP_SIZE; groupIndex++) {
            if (groupIndex != entityGroupsSel.selectedIndex()) {
                entityEntriesSel.childSel(groupIndex).clearSelection();
            }
        }
    }

    entityEntriesSel.update();
}

grid<uint8_t>& RoomEditorData::map()
{
    return data.map;
}

void RoomEditorData::mapTilesPlaced(const urect r)
{
    assert(data.map.size().contains(r));

    GridActions<AP::Map>::gridTilesPlaced(this, r);
}

void RoomEditorData::selectedTilesetTilesChanged()
{
    selectedTiles.clear();
    selectedScratchpadTiles.clear();
}

void RoomEditorData::selectedTilesChanged()
{
    selectedTilesetTiles.clear();
    selectedScratchpadTiles.clear();
}

void RoomEditorData::selectedScratchpadTilesChanged()
{
    selectedTiles.clear();
    selectedTilesetTiles.clear();
}

void RoomEditorData::clearSelectedTiles()
{
    selectedTilesetTiles.clear();
    selectedTiles.clear();
    selectedScratchpadTiles.clear();
}

RoomEditorGui::RoomEditorGui()
    : AbstractMetaTileEditorGui()
    , _data(nullptr)
    , _scratchpadTilemap()
    , _scratchpadRenderData()
    , _mapSize()
    , _graphics()
    , _entityTexture()
    , _entityGraphics(nullptr)
    , _scratchpadTilemapValid(false)
    , _tilesetAndPaletteIndexValid(false)
{
}

bool RoomEditorGui::setEditorData(AbstractEditorData* data)
{
    AbstractMetaTileEditorGui::setEditorData(data);
    return (_data = dynamic_cast<RoomEditorData*>(data));
}

void RoomEditorGui::editorDataChanged()
{
    AbstractMetaTileEditorGui::resetState();

    _scratchpadTilemapValid = false;
    _tilesetAndPaletteIndexValid = false;

    if (_data) {
        _mapSize = _data->data.map.size();
    }
}

void RoomEditorGui::editorOpened()
{
    AbstractMetaTileEditorGui::editorOpened();

    setEditMode(EditMode::SelectObjects);
}

void RoomEditorGui::editorClosed()
{
    AbstractMetaTileEditorGui::editorClosed();

    // ::TODO find a way to save scrollbar positions and restore them when the editor is opened again::
}

void RoomEditorGui::propertiesWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& room = _data->data;

    const static usize minMapSize(RM::RoomInput::MIN_MAP_WIDTH, RM::RoomInput::MIN_MAP_HEIGHT);
    const static usize maxMapSize(RM::RoomInput::MAX_MAP_WIDTH, RM::RoomInput::MAX_MAP_HEIGHT);

    if (ImGui::Begin("Room Properties")) {
        ImGui::SetWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

        ImGui::InputIdstring("Name", &room.name);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::name>(_data);
        }

        if (ImGui::IdStringCombo("Scene", &room.scene, projectFile.resourceScenes.scenes)) {
            _tilesetAndPaletteIndexValid = false;

            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::scene>(_data);
        }

        if (ImGui::InputUsize("Map Size", &_mapSize, maxMapSize)) {
            _mapSize.width = std::max(_mapSize.width, minMapSize.width);
            _mapSize.height = std::max(_mapSize.height, minMapSize.height);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            GridActions<AP::Map>::resizeGrid(_data, _mapSize);
            markTilemapOutOfDate();
        }
        if (!ImGui::IsItemActive()) {
            // ::TODO use callback to update scratchpad size::
            _mapSize = room.map.size();
        }
    }

    ImGui::End();
}

void RoomEditorGui::entrancesWindow()
{
    assert(_data);
    auto& room = _data->data;

    if (ImGui::Begin("Entrances##Room", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::SetWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);

        ListButtons<AP::Entrances>(_data);

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

        const usize bounds(room.mapRight(), room.mapBottom());

        for (unsigned i = 0; i < room.entrances.size(); i++) {
            auto& en = room.entrances.at(i);

            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->entrancesSel, i);
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
                ListActions<AP::Entrances>::itemEdited(_data, i);
            }

            ImGui::PopID();
        }

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::EndChild();
    }
    ImGui::End();
}

void RoomEditorGui::roomEntitiesWindow(const Project::ProjectFile& projectFile)
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

    assert(_data);
    auto& room = _data->data;

    if (ImGui::Begin("Entities##Room_Entities", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::SetWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);

        const rect bounds = room.validEntityArea();

        ImGui::PushID("Entity Groups");
        ListButtons<AP::EntityGroups>(_data);
        ImGui::PopID();
        ImGui::PushID("Entity Entries");
        ListButtons<AP::EntityEntries>(_data);
        ImGui::PopID();

        ImGui::BeginChild("Scroll");

        // ::TODO scroll to selected entity/group when selection changes::

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

        unsigned entityId = 0;

        const unsigned nGroups = std::min(room.entityGroups.size(), entityGroupNames.size());
        for (unsigned groupIndex = 0; groupIndex < nGroups; groupIndex++) {
            auto& group = room.entityGroups.at(groupIndex);

            if (ImGui::TreeNodeToggleSelection(entityGroupNames.at(groupIndex), &_data->entityGroupsSel, groupIndex)) {
                {
                    bool edited = false;

                    ImGui::InputIdstring("Name", &group.name);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    if (edited) {
                        ListActions<AP::EntityGroups>::itemEdited(_data, groupIndex);
                    }
                }
                ImGui::Spacing();

                for (unsigned i = 0; i < group.entities.size(); i++) {
                    auto& entity = group.entities.at(i);

                    bool edited = false;

                    ImGui::PushID(i);

                    const std::string selLabel = "Entity "s + std::to_string(entityId);
                    entityId++;

                    ImGui::Selectable(selLabel.c_str(), &_data->entityEntriesSel, groupIndex, i);

                    ImGui::Indent();

                    ImGui::InputIdstring("Name", &entity.name);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    edited |= ImGui::IdStringCombo("Entity Id", &entity.entityId, projectFile.entityRomData.entities);

                    ImGui::InputPoint("Position", &entity.position, bounds);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    ImGui::InputText("Parameter", &entity.parameter);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    if (edited) {
                        ListActions<AP::EntityEntries>::itemEdited(_data, groupIndex, i);
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

void RoomEditorGui::drawObjects(ImDrawList* drawList)
{
    assert(_entityGraphics);
    assert(_data);
    auto& room = _data->data;

    // ::TODO dynamic playerIds::
    constexpr unsigned playerId = 0;

    const ImTextureID textureId = _entityTexture.imguiTextureId();

    // NOTE: pushing a textureId to the drawlist prevents me from drawing filled rects
    drawList->PushTextureID(textureId);

    // ::TODO show/hide entrances::
    if (true) {
        for (const auto& entrance : room.entrances) {
            _graphics.drawEntity(drawList, &entrance.position,
                                 textureId, _entityGraphics->settingsForPlayer(playerId), IM_COL32_WHITE);
        }
    }

    // ::TODO show/hide entities::
    if (true) {
        const unsigned nGroups = std::min<size_t>(_data->entityEntriesSel.MAX_GROUP_SIZE, room.entityGroups.size());
        for (unsigned groupIndex = 0; groupIndex < nGroups; groupIndex++) {
            const auto& group = room.entityGroups.at(groupIndex);

            const bool groupEnabled = _data->entityGroupsSel.selectedIndex() > nGroups || _data->entityGroupsSel.selectedIndex() == groupIndex;

            const ImU32 tint = groupEnabled ? IM_COL32_WHITE : Style::disabledEntityGroupTint;

            for (const auto& entity : group.entities) {
                _graphics.drawEntity(drawList, &entity.position,
                                     textureId, _entityGraphics->settingsForEntity(entity.entityId),
                                     tint);
            }
        }
    }
}

void RoomEditorGui::drawAndEditObjects(ImDrawList* drawList)
{
    assert(_entityGraphics);
    assert(_data);
    auto& room = _data->data;

    // ::TODO dynamic playerIds::
    constexpr unsigned playerId = 0;

    const ImTextureID textureId = _entityTexture.imguiTextureId();
    // NOTE: Cannot push textureId to drawList - it prevents me from drawing filled rects.

    // ::TODO show/hide entrances::
    if (true) {
        for (unsigned i = 0; i < room.entrances.size(); i++) {
            auto& entrance = room.entrances.at(i);

            _graphics.addEntity(drawList, &entrance.position,
                                textureId, _entityGraphics->settingsForPlayer(playerId),
                                Style::entranceFillColor, Style::entranceOutlineColor, IM_COL32_WHITE,
                                &_data->entrancesSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::BeginTooltip();
                ImGui::Text("Extrance %u %s", i, entrance.name.str().c_str());
                ImGui::EndTooltip();
            }
        }
    }

    // ::TODO show/hide entities::
    if (true) {
        const unsigned nGroups = std::min<size_t>(_data->entityEntriesSel.MAX_GROUP_SIZE, room.entityGroups.size());
        for (unsigned groupIndex = 0; groupIndex < nGroups; groupIndex++) {
            auto& group = room.entityGroups.at(groupIndex);
            auto& childSel = _data->entityEntriesSel.childSel(groupIndex);

            if (_data->entityGroupsSel.selectedIndex() > nGroups || _data->entityGroupsSel.selectedIndex() == groupIndex) {
                // No entity groups are selected or the  groupIndex is the selected group

                for (unsigned i = 0; i < group.entities.size(); i++) {
                    auto& entity = group.entities.at(i);

                    _graphics.addEntity(drawList, &entity.position,
                                        textureId, _entityGraphics->settingsForEntity(entity.entityId),
                                        Style::entityFillColor, Style::entityOutlineColor, IM_COL32_WHITE,
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
                                         Style::disabledEntityGroupTint);
                }
            }
        }
    }
}

void RoomEditorGui::editorWindow()
{
    assert(_data);
    auto& room = _data->data;

    // ::TODO shrink entity vertical spacing at the top of the map::

    if (ImGui::Begin("Room###Room_Editor")) {
        ImGui::SetWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);

        // ::TODO expand toolbar::
        ImGui::TextUnformatted("::TODO expand toolBar::");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(180);
        ImGui::SingleSelectionNamedListCombo("##EntityGroupCombo", &_data->entityGroupsSel, room.entityGroups, true);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Selected Entity Group");
            ImGui::EndTooltip();
        }

        undoStackButtons();
        ImGui::SameLine(0.0f, 12.0f);

        if (selectObjectsButton()) {
            _data->clearSelectedTiles();
            selectionChanged();
        }
        ImGui::SameLine();
        editModeButtons();
        ImGui::SameLine(0.0f, 12.0f);

        showLayerButtons();
        ImGui::SameLine();

        Style::roomEditorZoom.zoomCombo("##zoom");

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const ImVec2& zoom = Style::roomEditorZoom.zoom();
        const rect bounds = room.validEntityArea();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        _graphics.setDisabled(editMode() != EditMode::SelectObjects);

        _graphics.startLoop("Room", bounds, zoom,
                            &_data->entrancesSel, &_data->entityEntriesSel);

        // ::TODO set initial scroll to position top-left of map ::

        const Geometry geo{
            ImVec2(METATILE_SIZE_PX * zoom.x, METATILE_SIZE_PX * zoom.y),
            ImVec2(room.map.width() * METATILE_SIZE_PX * zoom.x, room.map.height() * METATILE_SIZE_PX * zoom.y),
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
                          &_data->entrancesSel, &_data->entityEntriesSel);

        if (_graphics.isEditingFinished()) {
            _data->startMacro();

            ListActions<AP::Entrances>::selectedItemsEdited(_data);
            ListActions<AP::EntityEntries>::selectedItemsEdited(_data);

            _data->endMacro();
        }

        Style::roomEditorZoom.processMouseWheel();

        ImGui::EndChild();
    }
    ImGui::End();
}

void RoomEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData&)
{
    if (_data == nullptr) {
        return;
    }

    updateEntityGraphics();
    updateTilesetAndPaletteIndex(projectFile);
    updateTilemapAndTextures(projectFile);

    const grid<uint8_t>* scratchpad = nullptr;
    if (tilesetIndex() < projectFile.metaTileTilesets.size()) {
        if (auto* mt = projectFile.metaTileTilesets.at(tilesetIndex())) {
            scratchpad = &mt->scratchpad;

            updateScratchpadTilemap(*scratchpad);
        }
    }

    propertiesWindow(projectFile);
    entrancesWindow();
    roomEntitiesWindow(projectFile);

    editorWindow();

    tilesetMinimapWindow("Tileset##Room");

    minimapWindow("Minimap##Room");

    if (scratchpadMinimapWindow("Scratchpad##Room", &_scratchpadRenderData, _scratchpadTilemap, scratchpad, &_data->selectedScratchpadTiles)) {
        _data->selectedScratchpadTilesChanged();
        selectionChanged();
    }
}

void RoomEditorGui::selectionChanged()
{
}

void RoomEditorGui::updateEntityGraphics()
{
    assert(_data);

    // ::TODO retrieve _entityGraphics from ProjectData::
    if (_entityGraphics != nullptr) {
        return;
    }

    // ::TODO populate EntityGraphics data::
    _entityGraphics = std::make_shared<EntityGraphics>();

    _entityTexture.replace(_entityGraphics->image());
}

void RoomEditorGui::updateTilesetAndPaletteIndex(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& room = _data->data;

    // ::TODO invalidate tilesetIndex when ResourceScenes is compiled::

    if (_tilesetAndPaletteIndexValid) {
        return;
    }

    unsigned tilesetIndex = INT_MAX;
    unsigned paletteIndex = INT_MAX;

    // ::TODO use compiled data to get MetaTile id and palette id::

    if (auto scene = projectFile.resourceScenes.scenes.find(room.scene)) {
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

    _scratchpadTilemapValid = false;

    setTilesetIndex(tilesetIndex);
    setPaletteIndex(paletteIndex);

    _tilesetAndPaletteIndexValid = true;
}

void RoomEditorGui::updateScratchpadTilemap(const grid<uint8_t>& scratchpad)
{
    assert(_data);

    if (_scratchpadTilemapValid) {
        return;
    }

    _scratchpadTilemap.setMapData(scratchpad);

    _scratchpadTilemapValid = true;
}

}
