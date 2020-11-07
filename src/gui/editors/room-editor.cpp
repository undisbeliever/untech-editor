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
#include "models/project/project-data.h"
#include "models/rooms/rooms-serializer.h"

namespace UnTech::Gui {

namespace RM = UnTech::Rooms;

// DragDrop Payload: unsigned int - index in _entityGraphics.entities
static const char* const entityDragDropId = "DND_Entity";

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;

unsigned RoomEditorGui::playerId = 0;

bool RoomEditorGui::showEntrances = true;
bool RoomEditorGui::showEntities = true;

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

        const static usize MIN_SIZE;
        const static usize MAX_SIZE;
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

const usize RoomEditorData::AP::Map::MIN_SIZE(RM::RoomInput::MIN_MAP_WIDTH, RM::RoomInput::MIN_MAP_HEIGHT);
const usize RoomEditorData::AP::Map::MAX_SIZE(RM::RoomInput::MAX_MAP_WIDTH, RM::RoomInput::MAX_MAP_HEIGHT);

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
    , _tileFunctionTables()
    , _mapSize()
    , _graphics()
    , _entityTexture()
    , _entityGraphics(nullptr)
    , _scenesData(nullptr)
    , _mtTilesetValid(false)
    , _entityTextureWindowOpen(false)
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

    _mtTilesetValid = false;
    _scenesData = nullptr;

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
}

void RoomEditorGui::propertiesWindow(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& room = _data->data;

    ImGui::SetNextWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Room Properties")) {

        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

        ImGui::InputIdstring("Name", &room.name);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::name>(_data);
        }

        if (ImGui::IdStringCombo("Scene", &room.scene, projectFile.resourceScenes.scenes)) {
            _mtTilesetValid = false;

            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::scene>(_data);
        }

        if (ImGui::InputUsize("Map Size", &_mapSize, AP::Map::MAX_SIZE)) {
            _mapSize.width = std::max(_mapSize.width, AP::Map::MIN_SIZE.width);
            _mapSize.height = std::max(_mapSize.height, AP::Map::MIN_SIZE.height);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            GridActions<AP::Map>::resizeGrid(_data, _mapSize);
            markTilemapOutOfDate();
        }
    }

    ImGui::End();
}

void RoomEditorGui::entrancesWindow()
{
    assert(_data);
    auto& room = _data->data;

    ImGui::SetNextWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Entrances##Room", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

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

    ImGui::SetNextWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Entities##Room_Entities", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

        const rect bounds = room.validEntityArea();

        ImGui::PushID("Entity Groups");
        ListButtons<AP::EntityGroups>(_data);
        ImGui::PopID();
        ImGui::PushID("Entity Entries");
        ListButtons<AP::EntityEntries>(_data);
        ImGui::PopID();

        ImGui::BeginChild("Scroll");

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

void RoomEditorGui::entityTextureWindow()
{
    if (!_entityTextureWindowOpen) {
        return;
    }

    if (ImGui::Begin("Entity Texture##Room", &_entityTextureWindowOpen)) {
        ImGui::Text("Entity Texture (%u x %u pixels)", _entityTexture.width(), _entityTexture.height());

        const ImVec2 size(_entityTexture.width() * 2, _entityTexture.height() * 2);

        ImGui::Image(_entityTexture.imguiTextureId(), size);

        ImGui::End();
    }
}

void RoomEditorGui::entitiesWindow()
{
    assert(_entityGraphics);
    assert(_data);

    static ImGuiTextFilter filter;

    constexpr float buttonSize = 64.0f;

    ImGui::SetNextWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Entities##Room")) {

        const auto& style = ImGui::GetStyle();

        const auto textureId = _entityTexture.imguiTextureId();

        filter.Draw("Filter");

        ImGui::BeginChild("Scroll");

        const unsigned nColumns = std::floor((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x * 2) / (buttonSize + style.ItemSpacing.x));
        const ImVec2 size{ buttonSize, buttonSize };

        unsigned counter = 0;
        for (unsigned i = 0; i < _entityGraphics->entities.size(); i++) {
            const auto& eg = _entityGraphics->entities.at(i);

            if (filter.PassFilter(eg.name.str().c_str())) {
                ImGui::PushID(i);

                ImGui::ImageButton(textureId, size, eg.uvMin, eg.uvMax);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    ImGui::SetDragDropPayload(entityDragDropId, &i, sizeof(decltype(i)));

                    ImGui::TextUnformatted(eg.name);
                    ImGui::EndDragDropSource();
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::TextUnformatted(eg.name);
                    ImGui::EndTooltip();
                }

                ImGui::PopID();

                if ((++counter) != nColumns) {
                    ImGui::SameLine();
                }
                else {
                    counter = 0;
                }
            }
        }

        ImGui::EndChild();

        ImGui::End();
    }
}

void RoomEditorGui::entityDropTarget(ImDrawList* drawList)
{
    assert(_entityGraphics);
    assert(_data);
    const auto& room = _data->data;

    if (room.entityGroups.empty()) {
        return;
    }

    if (ImGui::BeginDragDropTarget()) {
        const auto flags = ImGuiDragDropFlags_AcceptNoDrawDefaultRect | ImGuiDragDropFlags_AcceptBeforeDelivery;

        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(entityDragDropId, flags)) {
            IM_ASSERT(payload->DataSize == sizeof(unsigned));

            unsigned entityIndex = *(const unsigned*)payload->Data;

            if (entityIndex < _entityGraphics->entities.size()) {
                const auto& ds = _entityGraphics->entities.at(entityIndex);

                const auto pos = _graphics.mousePos();

                _graphics.drawEntity(drawList, &pos, _entityTexture.imguiTextureId(), ds, IM_COL32_WHITE);

                if (payload->IsDelivery()) {
                    const unsigned groupIndex = _data->entityGroupsSel.hasSelection() ? _data->entityGroupsSel.selectedIndex() : 0;

                    UnTech::Rooms::EntityEntry entry;
                    entry.entityId = ds.name;
                    entry.position = pos;

                    ListActions<AP::EntityEntries>::addItem(_data, groupIndex, entry);

                    showEntities = true;
                }
            }
        }

        ImGui::EndDragDropTarget();
    }
}

void RoomEditorGui::drawObjects(ImDrawList* drawList)
{
    assert(_entityGraphics);
    assert(_data);
    auto& room = _data->data;

    const ImTextureID textureId = _entityTexture.imguiTextureId();

    // NOTE: pushing a textureId to the drawlist prevents me from drawing filled rects
    drawList->PushTextureID(textureId);

    if (showEntrances) {
        for (const auto& entrance : room.entrances) {
            _graphics.drawEntity(drawList, &entrance.position,
                                 textureId, _entityGraphics->settingsForPlayer(playerId), IM_COL32_WHITE);
        }
    }

    if (showEntities) {
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

    const ImTextureID textureId = _entityTexture.imguiTextureId();
    // NOTE: Cannot push textureId to drawList - it prevents me from drawing filled rects.

    if (showEntrances) {
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

    if (showEntities) {
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
    static constexpr const char* windowTitle = "Room###Room_Editor";

    assert(_entityGraphics);
    assert(_data);
    auto& room = _data->data;

    ImGui::SetNextWindowSize(ImVec2(600, 600), ImGuiCond_FirstUseEver);

    if (room.map.empty()) {
        ImGui::Begin(windowTitle);
        ImGui::End();
        return;
    }

    if (ImGui::Begin(windowTitle)) {
        animationButtons();
        ImGui::SameLine(0.0f, 12.0f);

        ImGui::SetNextItemWidth(180);
        ImGui::SingleSelectionNamedListCombo("##EntityGroupCombo", &_data->entityGroupsSel, room.entityGroups, true);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Selected Entity Group");
            ImGui::EndTooltip();
        }
        ImGui::SameLine(0.0f, 12.0f);

        ImGui::SetNextItemWidth(180);
        ImGui::SingleSelectionNamedListCombo("##PlayerId", &playerId, _entityGraphics->players, false);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted("Player Entity");
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
        ImGui::ToggledButtonWithTooltip("E##showEntities", &showEntities, "Show Entities");
        ImGui::SameLine();
        ImGui::ToggledButtonWithTooltip("P##showEntrances", &showEntrances, "Show Player Entrances");
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

        // Draw drag+drop entity on top of selected entity outlines
        entityDropTarget(drawList);

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

void RoomEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData)
{
    if (_data == nullptr) {
        return;
    }

    updateMapAndProcessAnimations();
    updateEntityGraphics();
    updateTilesetData(projectFile, projectData);

    propertiesWindow(projectFile);
    entrancesWindow();
    entitiesWindow();
    roomEntitiesWindow(projectFile);

    editorWindow();

    tilesetMinimapWindow("Tileset##Room");

    minimapWindow("Minimap##Room");

    if (scratchpadMinimapWindow("Scratchpad##Room", _scratchpadTilemap, _scratchpad, &_data->selectedScratchpadTiles)) {
        _data->selectedScratchpadTilesChanged();
        selectionChanged();
    }

    entityTextureWindow();
}

void RoomEditorGui::viewMenu()
{
    AbstractMetaTileEditorGui::viewMenu();

    ImGui::MenuItem("Show Entities", nullptr, &showEntities);
    ImGui::MenuItem("Show Player Entrances", nullptr, &showEntrances);

    ImGui::Separator();

    ImGui::MenuItem("Show Entity Texture", nullptr, &_entityTextureWindowOpen);
}

void RoomEditorGui::selectionChanged()
{
}

const std::array<idstring, 256>& RoomEditorGui::tileFunctionTables() const
{
    return _tileFunctionTables;
}

void RoomEditorGui::updateEntityGraphics()
{
    assert(_data);

    auto eg = entityGraphicsStore.get();

    if (_entityGraphics != eg) {
        assert(eg != nullptr);

        _entityTexture.replace(eg->image);

        _entityGraphics = std::move(eg);

        if (playerId >= _entityGraphics->players.size()) {
            playerId = 0;
        }
    }
}

void RoomEditorGui::updateTilesetData(const Project::ProjectFile& projectFile,
                                      const Project::ProjectData& projectData)
{
    assert(_data);
    auto& room = _data->data;

    const auto scenes = projectData.scenes();
    if (_scenesData != scenes) {
        _scenesData = scenes;
        _mtTilesetValid = false;
    }

    if (_mtTilesetValid) {
        return;
    }

    unsigned tilesetIndex = INT_MAX;
    unsigned paletteIndex = INT_MAX;

    if (scenes) {
        if (auto s = scenes->findScene(room.scene)) {
            tilesetIndex = s->mtTileset.value_or(INT_MAX);
            paletteIndex = s->palette.value_or(INT_MAX);
        }
    }
    else {
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
    }

    _tilesetShader.setPaletteData(projectData.palettes().at(paletteIndex));

    const auto mtData = projectData.metaTileTilesets().at(tilesetIndex);
    const bool mtDataChanged = mtData != _tilesetShader.tilesetData();

    if (mtDataChanged || !mtData) {
        const auto* tileset = tilesetIndex < projectFile.metaTileTilesets.size()
                                  ? projectFile.metaTileTilesets.at(tilesetIndex)
                                  : nullptr;

        if (tileset) {
            _scratchpad = tileset->scratchpad;
        }
        else {
            _scratchpad = grid<uint8_t>(0, 0);
        }
        _scratchpadTilemap.setMapData(_scratchpad);

        if (tileset) {
            _tilesetShader.setTilesetData(*tileset, std::move(mtData));
            _tilesetShader.setTileCollisions(tileset->tileCollisions);
            _tilesetShader.setInteractiveTilesData(*tileset, projectFile, projectData);

            _tileFunctionTables = tileset->tileFunctionTables;
        }
        else {
            _tilesetShader.reset();

            for (auto& tft : _tileFunctionTables) {
                tft.clear();
            }
        }
    }

    _mtTilesetValid = true;
}

}
