/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "room-editor.h"
#include "gui/aptable.h"
#include "gui/editor-actions.h"
#include "gui/graphics/aabb-graphics.h"
#include "gui/graphics/entity-graphics.h"
#include "gui/grid-actions.h"
#include "gui/splitter.hpp"
#include "gui/style.h"
#include "models/common/iterators.h"
#include "models/project/project-data.h"
#include "models/rooms/room-error.h"
#include "models/rooms/rooms-serializer.h"
#include "models/scripting/scripting-error.h"
#include "vendor/imgui/imgui.h"

namespace UnTech::Gui {

namespace RM = UnTech::Rooms;

// DragDrop Payload: index in _entityGraphics.entities
static const char* const entityDragDropId = "DND_Entity";
using EntityDragDropType = size_t;

static constexpr unsigned METATILE_SIZE_PX = MetaTiles::METATILE_SIZE_PX;

unsigned RoomEditorGui::playerId = 0;

bool RoomEditorGui::showEntrances = true;
bool RoomEditorGui::showEntities = true;
bool RoomEditorGui::showScriptTriggers = true;

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

    struct Scene : public Room {
        constexpr static auto FieldPtr = &EditorDataT::scene;
        constexpr static auto validFlag = &RoomEditorGui::_mtTilesetValid;
    };

    struct Map : public Room {
        using GridT = grid<uint8_t>;
        using ListArgsT = std::tuple<>;

        const static usize MIN_SIZE;
        const static usize MAX_SIZE;
        constexpr static uint8_t DEFAULT_VALUE = 0;

        constexpr static auto SelectionPtr = &EditorT::selectedTiles;

        constexpr static auto validFlag = &RoomEditorGui::_tilemapValid;

        // cppcheck-suppress unusedFunction
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

    struct ScriptTriggers final : public Room {
        using ListT = std::vector<UnTech::Rooms::ScriptTrigger>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = RM::MAX_SCRIPT_TRIGGERS;

        constexpr static auto SelectionPtr = &EditorT::scriptTriggersSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.scriptTriggers; }
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

// ::HACK Reading through the script line by line is easier then adding NodeSelection::ParentIndexT to the script compiler ::
static void selectScriptLine(NodeSelection& sel, const Scripting::Script& script, const unsigned scriptIndex, const unsigned lineNo)
{
    using namespace UnTech::Scripting;

    struct Visitor {
        unsigned targetLineNo;

        unsigned lineNo = 0;

        unsigned depth = 0;
        unsigned index = 0;
        std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> parentIndex{};

        std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> foundParentIndex{};
        unsigned foundChildIndex = INT_MAX;

        explicit Visitor(const unsigned targetLineNo)
            : targetLineNo(targetLineNo)
        {
        }

        void readScript(const Scripting::Script& script, const unsigned scriptIndex)
        {
            lineNo = 0;
            depth = 0;
            parentIndex.fill(0xffff);
            foundParentIndex.fill(0xffff);
            foundChildIndex = 0xffff;

            index = scriptIndex;

            readStatements(script.statements, false);
        }

        void readStatements(const std::vector<ScriptNode>& statements, const bool isFalseBranch)
        {
            if (depth >= parentIndex.size()) {
                return;
            }

            // Save index - prevents an infinite loop
            const unsigned oldIndex = index;

            parentIndex.at(depth) = index;
            if (isFalseBranch) {
                parentIndex.at(depth) |= 0x8000;
            }
            depth++;

            for (auto [i, s] : enumerate(statements)) {
                // Putting mask here allows StartScript line selection to work correctly
                index = i & 0x7fff;

                lineNo++;

                if (lineNo == targetLineNo) {
                    foundParentIndex = parentIndex;
                    foundChildIndex = i;
                    return;
                }

                std::visit(*this, s);
            }

            // restore index
            index = oldIndex;

            depth--;
            parentIndex.at(depth) = NodeSelection::NO_SELECTION;
        }

        void operator()(const Statement&) const
        {
        }

        void operator()(const IfStatement& s)
        {
            readStatements(s.thenStatements, false);

            if (!s.elseStatements.empty()) {
                readStatements(s.elseStatements, true);
            }
        }

        void operator()(const WhileStatement& s)
        {
            readStatements(s.statements, false);
        }

        void operator()(const Comment&) const
        {
        }
    };

    Visitor visitor(lineNo);

    visitor.readScript(script, scriptIndex);

    sel.setSelected(visitor.foundParentIndex, visitor.foundChildIndex);
}

void RoomEditorData::errorDoubleClicked(const AbstractError* error)
{
    selectedTilesetTiles.clear();
    selectedTiles.clear();
    selectedScratchpadTiles.clear();

    entrancesSel.clearSelection();
    entityGroupsSel.clearSelection();
    entityEntriesSel.clearSelection();
    scriptTriggersSel.clearSelection();

    tempScriptFlagsSel.clearSelection();
    tempScriptWordsSel.clearSelection();

    scriptsSel.clearSelection();
    scriptStatementsSel.clearSelection();

    if (auto* e = dynamic_cast<const Rooms::RoomError*>(error)) {
        using Type = Rooms::RoomErrorType;

        switch (e->type) {
        case Type::ROOM_ENTRANCE:
            entrancesSel.setSelected(e->firstIndex);
            break;

        case Type::ENTITY_GROUP:
            entityGroupsSel.setSelected(e->firstIndex);
            break;

        case Type::ENTITY_ENTRY:
            entityGroupsSel.setSelected(e->firstIndex);
            entityEntriesSel.setSelected(e->firstIndex, e->childIndex);
            break;

        case Type::SCRIPT_TRIGGER:
            scriptTriggersSel.setSelected(e->firstIndex);
            break;
        }
    }

    if (auto* e = dynamic_cast<const Scripting::ScriptError*>(error)) {
        using Type = Scripting::ScriptErrorType;

        switch (e->type) {
        case Type::TEMP_FLAG:
            tempScriptFlagsSel.setSelected(e->firstIndex);
            break;

        case Type::TEMP_WORD:
            tempScriptWordsSel.setSelected(e->firstIndex);
            break;

        case Type::SCRIPT:
            scriptsSel.setSelected(e->firstIndex);
            break;

        case Type::SCRIPT_LINE:
            scriptsSel.setSelected(e->firstIndex);
            if (e->firstIndex < data.roomScripts.scripts.size()) {
                selectScriptLine(scriptStatementsSel, data.roomScripts.scripts.at(e->firstIndex), e->firstIndex, e->childIndex);
            }
            else {
                selectScriptLine(scriptStatementsSel, data.roomScripts.startupScript, NodeSelection::NO_SELECTION, e->childIndex);
            }
            break;
        }
    }
}

void RoomEditorData::updateSelection()
{
    entrancesSel.update();
    entityGroupsSel.update();

    if (entityGroupsSel.hasSelection()) {
        // Disable entity selection if the group is not selected.
        for (const auto groupIndex : range(entityEntriesSel.MAX_GROUP_SIZE)) {
            if (groupIndex != entityGroupsSel.selectedIndex()) {
                entityEntriesSel.childSel(groupIndex).clearSelection();
            }
        }
    }

    entityEntriesSel.update();

    scriptTriggersSel.update();

    tempScriptFlagsSel.update();
    tempScriptWordsSel.update();

    scriptsSel.update();

    const unsigned selectedScriptIndex = std::min(scriptsSel.selectedIndex(), NodeSelection::NO_SELECTION);
    if (scriptStatementsSel.pendingParentIndex().front() != selectedScriptIndex) {
        NodeSelection::ParentIndexT pIndex;
        pIndex.fill(NodeSelection::NO_SELECTION);
        pIndex.at(0) = selectedScriptIndex;

        scriptStatementsSel.setParentIndex(pIndex);
    }
    scriptStatementsSel.update();
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
    : AbstractMetaTileEditorGui("##Room editor")
    , _data(nullptr)
    , _roomScriptGui()
    , _scratchpadTilemap()
    , _tileFunctionTables()
    , _mapSize()
    , _graphics()
    , _entityTexture()
    , _entityGraphics(nullptr)
    , _scenesData(nullptr)
    , _sidebar{ 360, 300, 300 }
    , _minimapRight_sidebar{ 350, 280, 400 }
    , _minimapRight_bottombar{ 350, 100, 100 }
    , _minimapBottom_sidebar{ 350, 280, 400 }
    , _minimapBottom_rightbar{ 350, 100, 100 }
    , _entitiesDropdownWindowPos{}
    , _showEntitiesDropdownWindow(false)
    , _minimapOnRight(true)
    , _entityTextureWindowOpen(false)
    , _mtTilesetValid(false)
{
}

bool RoomEditorGui::setEditorData(const std::shared_ptr<AbstractEditorData>& data)
{
    AbstractMetaTileEditorGui::setEditorData(data);
    _data = std::dynamic_pointer_cast<RoomEditorData>(data);

    _roomScriptGui.setEditorData(_data);

    return _data != nullptr;
}

void RoomEditorGui::resetState()
{
    AbstractMetaTileEditorGui::resetState();

    setEditMode(EditMode::SelectObjects);

    _mtTilesetValid = false;
    _scenesData = nullptr;

    _showEntitiesDropdownWindow = false;
}

void RoomEditorGui::editorClosed()
{
    AbstractMetaTileEditorGui::editorClosed();
}

grid<uint8_t>& RoomEditorGui::map()
{
    assert(_data);
    return _data->data.map;
}

void RoomEditorGui::mapTilesPlaced(const urect r)
{
    assert(_data);
    const auto& room = _data->data;

    assert(room.map.size().contains(r));

    GridActions<AP::Map>::gridTilesPlaced(_data, r);
}

void RoomEditorGui::selectedTilesetTilesChanged()
{
    assert(_data);

    _data->selectedTiles.clear();
    _data->selectedScratchpadTiles.clear();
}

void RoomEditorGui::selectedTilesChanged()
{
    assert(_data);

    _data->selectedTilesetTiles.clear();
    _data->selectedScratchpadTiles.clear();
}

constexpr static std::array<const char8_t*, RM::MAX_ENTITY_GROUPS + 4> entityGroupText{
    u8"Entity Group 0:",
    u8"Entity Group 1:",
    u8"Entity Group 2:",
    u8"Entity Group 3:",
    u8"Entity Group 4:",
    u8"Entity Group 5:",
    u8"Entity Group 6:",
    u8"Entity Group 7:",
    u8"Entity Group OUT OF BOUNDS:",
    u8"Entity Group OUT OF BOUNDS:",
    u8"Entity Group OUT OF BOUNDS:",
    u8"Entity Group OUT OF BOUNDS:",
};

void RoomEditorGui::propertiesGui(const Project::ProjectFile& projectFile)
{
    assert(_data);
    auto& room = _data->data;

    const usize mapSize(room.map.size());
    const usize mapSizePx(room.mapRight(), room.mapBottom());
    const rect entityArea = room.validEntityArea();

    const ImVec2 fixedTableSize(0, std::round(ImGui::GetFrameHeightWithSpacing() * 4.125f));

    {
        ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.5f);

        if (Cell("Name", &room.name)) {
            EditorActions<AP::Room>::fieldEdited<
                &RM::RoomInput::name>(_data);
        }

        if (Cell("Scene", &room.scene, projectFile.resourceScenes.scenes)) {
            EditorFieldActions<AP::Scene>::fieldEdited(_data);
        }

        if (Cell("Map Size", &_mapSize, AP::Map::MAX_SIZE)) {
            _mapSize.width = std::max(_mapSize.width, AP::Map::MIN_SIZE.width);
            _mapSize.height = std::max(_mapSize.height, AP::Map::MIN_SIZE.height);

            GridActions<AP::Map>::resizeGrid(_data, _mapSize);
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted(u8"Entrances:");

    apTable<AP::Entrances>(
        "Entrances", _data,
        std::to_array({ "Name", "Position", "Orientation" }),
        fixedTableSize,

        [&](auto& en) { return Cell("##name", &en.name); },
        [&](auto& en) { return Cell("##pos", &en.position, mapSizePx); },
        [&](auto& en) { return Cell("##orientation", &en.orientation); });

    ImGui::Separator();
    ImGui::TextUnformatted(u8"Script Triggers:");

    apTable<AP::ScriptTriggers>(
        "Table", _data,
        std::to_array({ "Script", "AABB", "Once" }),
        fixedTableSize,

        [&](auto& st) { return Cell("##script", &st.script, room.roomScripts.scripts); },
        [&](auto& st) { return Cell("##aabb", &st.aabb, mapSize); },
        [&](auto& st) { return Cell("Once", &st.once); });

    ImGui::Separator();
    ImGui::TextUnformatted(u8"Entities:");

    ImGui::PushID("Entity Groups");
    ListButtons<AP::EntityGroups>(_data);
    ImGui::PopID();
    ImGui::PushID("Entity Entries");
    ListButtons<AP::EntityEntries>(_data);
    ImGui::PopID();

    constexpr auto columnNames = std::to_array({ "Name\nPosition", "Entity Id\nParameter" });

    if (beginApTable("Entities", columnNames)) {
        unsigned entityId = 0;

        const unsigned nGroups = std::min(room.entityGroups.size(), entityGroupText.size());
        for (const auto groupIndex : range(nGroups)) {
            auto& group = room.entityGroups.at(groupIndex);

            ImGui::PushID(groupIndex);

            {
                bool edited = false;

                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                ImGui::Separator();
                ImGui::Selectable(&_data->entityGroupsSel, groupIndex);

                ImGui::TableNextColumn();
                ImGui::Separator();
                ImGui::TextUnformatted(entityGroupText.at(groupIndex));
                ImGui::NewLine();

                ImGui::TableNextColumn();
                ImGui::Separator();
                ImGui::SetNextItemWidth(-1);
                edited |= Cell("##name", &group.name);

                if (edited) {
                    ListActions<AP::EntityGroups>::itemEdited(_data, groupIndex);
                }
            }

            ImGui::Indent();

            apTable_data_custom<AP::EntityEntries>(
                _data,
                std::make_tuple(groupIndex),
                [&](auto* sel, auto index) {
                    const std::u8string selLabel = stringBuilder(entityId);
                    entityId++;
                    ImGui::Selectable(u8Cast(selLabel), sel, groupIndex, index);
                },

                [&](auto& ee) {
                    bool edited = false;
                    edited |= Cell("##name", &ee.name);
                    ImGui::SetNextItemWidth(-1);
                    edited |= Cell("##position", &ee.position, entityArea);
                    return edited;
                },
                [&](auto& ee) {
                    bool edited = false;
                    edited |= Cell("##entityId", &ee.entityId, projectFile.entityRomData.entities);
                    ImGui::SetNextItemWidth(-1);
                    edited |= Cell("##parameter", &ee.parameter);
                    return edited;
                });

            ImGui::Unindent();
            ImGui::Spacing();

            ImGui::PopID();
        }

        endApTable();
    }
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
    }

    ImGui::End();
}

void RoomEditorGui::entitiesDropdownWindow()
{
    assert(_entityGraphics);
    assert(_data);

    static ImGuiTextFilter filter;

    constexpr float buttonSize = 64.0f;

    constexpr auto windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;

    if (!_showEntitiesDropdownWindow) {
        return;
    }

    ImGui::SetNextWindowSize(ImVec2(350, 350), ImGuiCond_Once);
    ImGui::SetNextWindowPos(_entitiesDropdownWindowPos, ImGuiCond_Always);

    if (ImGui::Begin("Entities##Room", &_showEntitiesDropdownWindow, windowFlags)) {
        const auto& style = ImGui::GetStyle();

        const auto textureId = _entityTexture.imguiTextureId();

        filter.Draw("Filter");

        ImGui::BeginChild("Scroll");

        const unsigned nColumns = std::floor((ImGui::GetContentRegionAvail().x - style.ItemSpacing.x * 2) / (buttonSize + style.ItemSpacing.x));
        const ImVec2 size{ buttonSize, buttonSize };

        unsigned counter = 0;
        for (auto [i, eg] : const_enumerate(_entityGraphics->entities)) {
            if (filter.PassFilter(u8Cast(eg.name))) {
                ImGui::PushID(i);

                ImGui::ImageButton("##entity", textureId, size, eg.uvMin, eg.uvMax);
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    const EntityDragDropType payload = i;
                    ImGui::SetDragDropPayload(entityDragDropId, &payload, sizeof(decltype(payload)));

                    ImGui::TextUnformatted(eg.name);
                    ImGui::EndDragDropSource();
                }

                if (ImGui::IsItemHovered()) {
                    ImGui::ShowTooltip(eg.name);
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
    }

    ImGui::End();
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
            IM_ASSERT(payload->DataSize == sizeof(EntityDragDropType));

            static_assert(std::is_trivial_v<EntityDragDropType>);

            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            auto entityIndex = *reinterpret_cast<const EntityDragDropType*>(payload->Data);

            static_assert(std::is_unsigned_v<EntityDragDropType>);
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
        for (const auto groupIndex : range(nGroups)) {
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

    drawList->PopTextureID();

    if (showScriptTriggers) {
        for (const auto& st : room.scriptTriggers) {
            TwoPointRect r(st.aabb);
            r.x1 *= METATILE_SIZE_PX;
            r.x2 *= METATILE_SIZE_PX;
            r.y1 *= METATILE_SIZE_PX;
            r.y2 *= METATILE_SIZE_PX;

            _graphics.drawRectFilled(drawList, r, Style::scriptTriggerFillColor, Style::scriptTriggerOutlineColor);
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
        for (auto [i, entrance] : enumerate(room.entrances)) {
            _graphics.addEntity(drawList, &entrance.position,
                                textureId, _entityGraphics->settingsForPlayer(playerId),
                                Style::entranceFillColor, Style::entranceOutlineColor, IM_COL32_WHITE,
                                &_data->entrancesSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                ImGui::ShowTooltipFmt("Extrance %u %s", unsigned(i), u8Cast(entrance.name));
            }
        }
    }

    if (showEntities) {
        const unsigned nGroups = std::min<size_t>(_data->entityEntriesSel.MAX_GROUP_SIZE, room.entityGroups.size());
        for (const auto groupIndex : range(nGroups)) {
            auto& group = room.entityGroups.at(groupIndex);
            auto& childSel = _data->entityEntriesSel.childSel(groupIndex);

            if (_data->entityGroupsSel.selectedIndex() > nGroups || _data->entityGroupsSel.selectedIndex() == groupIndex) {
                // No entity groups are selected or the  groupIndex is the selected group

                for (auto [i, entity] : enumerate(group.entities)) {
                    _graphics.addEntity(drawList, &entity.position,
                                        textureId, _entityGraphics->settingsForEntity(entity.entityId),
                                        Style::entityFillColor, Style::entityOutlineColor, IM_COL32_WHITE,
                                        &childSel, i);
                    if (_graphics.isHoveredAndNotEditing()) {
                        if (ImGui::BeginTooltip()) {
                            ImGui::Text("Entity %u (%s)", unsigned(i), u8Cast(entity.entityId));
                            ImGui::Indent();
                            if (entity.name.isValid()) {
                                ImGui::Text("Name: %s", u8Cast(entity.name));
                            }
                            ImGui::Text("Group: %u (%s)", unsigned(groupIndex), u8Cast(group.name));
                            if (!entity.parameter.empty()) {
                                ImGui::Text("Parameter: %s", u8Cast(entity.parameter));
                            }
                            ImGui::Unindent();
                            ImGui::EndTooltip();
                        }
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

    if (showScriptTriggers) {
        const TwoPointRect bounds(0, room.map.width() * METATILE_SIZE_PX, 0, room.map.height() * METATILE_SIZE_PX);

        for (auto [i, st] : enumerate(room.scriptTriggers)) {
            _graphics.addScriptTriggerRect(drawList, &st.aabb, bounds,
                                           Style::scriptTriggerFillColor, Style::scriptTriggerOutlineColor,
                                           &_data->scriptTriggersSel, i);

            if (_graphics.isHoveredAndNotEditing()) {
                if (!st.once) {
                    ImGui::ShowTooltipFmt("Script Trigger %u: %s", unsigned(i), u8Cast(st.script));
                }
                else {
                    ImGui::ShowTooltipFmt("Script Trigger %u: %s (once)", unsigned(i), u8Cast(st.script));
                }
            }
        }
    }
}

void RoomEditorGui::editorGui()
{
    assert(_entityGraphics);
    assert(_data);
    auto& room = _data->data;

    if (!room.map.empty()) {
        animationButtons();
        ImGui::SameLine(0.0f, 12.0f);

        ImGui::SetNextItemWidth(180);
        ImGui::SingleSelectionNamedListCombo("##EntityGroupCombo", &_data->entityGroupsSel, room.entityGroups, true);
        if (ImGui::IsItemHovered()) {
            ImGui::ShowTooltip(u8"Selected Entity Group");
        }
        ImGui::SameLine(0.0f, 12.0f);

        ImGui::SetNextItemWidth(180);
        ImGui::SingleSelectionNamedListCombo("##PlayerId", &playerId, _entityGraphics->players, false);
        if (ImGui::IsItemHovered()) {
            ImGui::ShowTooltip(u8"Player Entity");
        }

        undoStackButtons();
        ImGui::SameLine(0.0f, 18.0f);

        // Update the position of the entities dropdown window
        _entitiesDropdownWindowPos = ImGui::GetCursorScreenPos() + ImVec2(0, ImGui::GetFrameHeightWithSpacing());
        ImGui::ToggledButton("Entities", &_showEntitiesDropdownWindow);
        ImGui::SameLine(0.0f, 18.0f);

        if (selectObjectsButton()) {
            _data->clearSelectedTiles();
            selectionChanged();
        }
        ImGui::SameLine();
        editModeButtons();
        ImGui::SameLine(0.0f, 18.0f);

        showLayerButtons();
        ImGui::SameLine();
        ImGui::ToggledButtonWithTooltip("E##showEntities", &showEntities, "Show Entities");
        ImGui::SameLine();
        ImGui::ToggledButtonWithTooltip("P##showEntrances", &showEntrances, "Show Player Entrances");
        ImGui::SameLine();
        ImGui::ToggledButtonWithTooltip("S##showScriptTriggers", &showScriptTriggers, "Show Script Triggers");
        ImGui::SameLine();

        Style::roomEditorZoom.zoomCombo("##zoom");
        ImGui::SameLine();

        ImGui::ToggledButtonWithTooltip("MR", &_minimapOnRight, "Minimap on Right");

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const ImVec2& zoom = Style::roomEditorZoom.zoom();
        const rect bounds = room.validEntityArea();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        _graphics.setDisabled(editMode() != EditMode::SelectObjects);

        _graphics.startLoop("Room", bounds, zoom,
                            &_data->entrancesSel, &_data->entityEntriesSel, &_data->scriptTriggersSel);

        // ::TODO set initial scroll to position top-left of map ::

        const ImVec2 offset = drawAndEditMap(_graphics);

        if (editMode() == EditMode::SelectObjects) {
            drawAndEditObjects(drawList);
        }
        else {
            drawObjects(drawList);
        }

        _graphics.endLoop(drawList,
                          &_data->entrancesSel, &_data->entityEntriesSel, &_data->scriptTriggersSel);

        _invalidTiles.draw(drawList, _graphics.zoom(), offset);

        // Draw drag+drop entity on top of selected entity outlines
        entityDropTarget(drawList);

        if (_graphics.isEditingFinished()) {
            _data->undoStack().startMacro();

            ListActions<AP::Entrances>::selectedItemsEdited(_data);
            ListActions<AP::EntityEntries>::selectedItemsEdited(_data);
            ListActions<AP::ScriptTriggers>::selectedItemsEdited(_data);

            _data->undoStack().endMacro();
        }

        Style::roomEditorZoom.processMouseWheel();

        ImGui::EndChild();
    }
}

void RoomEditorGui::scratchpadGui()
{
    assert(_data);

    if (scratchpadMinimapGui("Scratchpad", _scratchpadTilemap, _scratchpad, &_data->selectedScratchpadTiles)) {
        _data->selectedScratchpadTilesChanged();
        selectionChanged();
    }
}

void RoomEditorGui::processGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData)
{
    if (_data == nullptr) {
        return;
    }

    // Update mapSize if the map changed
    if (!_tilemapValid) {
        _mapSize = _data->data.map.size();
    }

    updateMapAndProcessAnimations();
    updateEntityGraphics();
    updateTilesetData(projectFile, projectData);

    splitterSidebarRight(
        "##splitter", &_sidebar,
        "##Content",
        [&] {
            if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
                if (ImGui::BeginTabItem("Room")) {
                    if (_minimapOnRight) {
                        splitterSidebarRight(
                            "##mmspRight", &_minimapRight_sidebar,
                            "##Editor",
                            [&] {
                                editorGui();
                            },
                            "##Tiles",
                            [&] {
                                ImGui::BeginChild("##tileset", ImVec2(0, 270), false);
                                drawTileset("##TS", ImVec2(1, 1));
                                ImGui::EndChild();

                                ImGui::Separator();

                                splitterBottombar(
                                    "##minimap splitter R", &_minimapRight_bottombar,
                                    "##scratchpad",
                                    [&]() {
                                        scratchpadGui();
                                    },
                                    "##minimap",
                                    [&]() {
                                        minimapGui("minimap");
                                    });
                            });
                    }
                    else {
                        splitterBottombar(
                            "##mmspBottom", &_minimapBottom_sidebar,
                            "##Editor",
                            [&] {
                                editorGui();
                            },
                            "##Tiles",
                            [&] {
                                ImGui::BeginChild("##tileset", ImVec2(270, 0), false);
                                drawTileset("##TS", ImVec2(1, 1));
                                ImGui::EndChild();
                                ImGui::SameLineWithSeparator();

                                splitterSidebarRight(
                                    "##minimap splitter B", &_minimapBottom_rightbar,
                                    "##scratchpad",
                                    [&]() {
                                        scratchpadGui();
                                    },
                                    "##minimap",
                                    [&]() {
                                        minimapGui("minimap");
                                    });
                            });
                    }
                    ImGui::EndTabItem();
                }
                if (ImGui::BeginTabItem("Script Editor")) {
                    _roomScriptGui.processGui(projectFile, projectData);

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
        },
        "##Sidebar",
        [&] {
            propertiesGui(projectFile);
        });
}

void RoomEditorGui::processExtraWindows(const Project::ProjectFile&, const Project::ProjectData&)
{
    entitiesDropdownWindow();
    entityTextureWindow();
}

void RoomEditorGui::viewMenu()
{
    AbstractMetaTileEditorGui::viewMenu();

    ImGui::MenuItem("Show Entities", nullptr, &showEntities);
    ImGui::MenuItem("Show Player Entrances", nullptr, &showEntrances);
    ImGui::MenuItem("Show Script Triggers", nullptr, &showScriptTriggers);

    ImGui::Separator();
    ImGui::MenuItem("Minimap on Right", nullptr, &_minimapOnRight);

    ImGui::Separator();
    ImGui::MenuItem("Show Entities Window", nullptr, &_showEntitiesDropdownWindow);

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

    const auto scenes = projectData.projectSettingsData.scenes();
    if (_scenesData != scenes) {
        _scenesData = scenes;
        _mtTilesetValid = false;
    }

    if (_mtTilesetValid) {
        return;
    }

    std::optional<unsigned> tilesetIndex{};
    std::optional<unsigned> paletteIndex{};

    if (scenes) {
        if (auto s = scenes->findScene(room.scene)) {
            tilesetIndex = s->mtTileset;
            paletteIndex = s->palette;
        }
    }
    else {
        if (auto scene = projectFile.resourceScenes.scenes.find(room.scene)) {
            paletteIndex = projectFile.palettes.indexOf(scene->palette);

            if (auto sceneSettings = projectFile.resourceScenes.settings.find(scene->sceneSettings)) {
                for (auto [layerId, layerType] : const_enumerate(sceneSettings->layerTypes)) {
                    if (layerType == UnTech::Resources::LayerType::MetaTileTileset) {
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

    const auto palData = paletteIndex ? projectData.palettes.at(paletteIndex.value())
                                      : nullptr;

    const auto mtData = tilesetIndex ? projectData.metaTileTilesets.at(tilesetIndex.value())
                                     : nullptr;

    _tilesetShader.setPaletteData(palData);

    const bool mtDataChanged = mtData != _tilesetShader.tilesetData();
    if (mtDataChanged || !mtData) {
        const auto tileset = tilesetIndex ? projectFile.metaTileTilesets.at(tilesetIndex.value())
                                          : std::nullopt;

        if (tileset) {
            _scratchpad = tileset->scratchpad;
        }
        else {
            _scratchpad = grid<uint8_t>(0, 0);
        }
        _scratchpadTilemap.setMapData(_scratchpad);

        if (tileset) {
            _tilesetShader.setTilesetData(*tileset, mtData);
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

void RoomEditorGui::resourceCompiled(const ErrorList& errors)
{
    assert(_data);

    _invalidTiles.clear();

    for (const auto& errorItem : errors.list()) {
        if (auto* tileErr = dynamic_cast<const Rooms::InvalidRoomTilesError*>(errorItem.get())) {
            _invalidTiles.append(*tileErr);
        }
    }
}

}
