/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "room-editor.h"
#include "gui/editor-actions.h"
#include "gui/graphics/aabb-graphics.h"
#include "gui/graphics/entity-graphics.h"
#include "gui/grid-actions.h"
#include "gui/imgui-combos.h"
#include "gui/imgui.h"
#include "gui/list-actions-variant.h"
#include "gui/list-actions.h"
#include "gui/list-helpers.h"
#include "gui/style.h"
#include "models/common/iterators.h"
#include "models/project/project-data.h"
#include "models/rooms/room-error.h"
#include "models/rooms/rooms-serializer.h"
#include "models/scripting/scripting-error.h"

namespace UnTech::Gui {

namespace RM = UnTech::Rooms;

// DragDrop Payload: unsigned int - index in _entityGraphics.entities
static const char* const entityDragDropId = "DND_Entity";

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

    struct TempScriptFlags final : public Room {
        using ListT = std::vector<idstring>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = MultipleSelection::MAX_SIZE;

        constexpr static auto SelectionPtr = &EditorT::tempScriptFlagsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.roomScripts.tempFlags; }
    };

    struct TempScriptWords final : public Room {
        using ListT = std::vector<idstring>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = MultipleSelection::MAX_SIZE;

        constexpr static auto SelectionPtr = &EditorT::tempScriptWordsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.roomScripts.tempWords; }
    };

    struct Scripts final : public Room {
        using ListT = NamedList<Scripting::Script>;
        using ListArgsT = std::tuple<>;
        using SelectionT = SingleSelection;

        constexpr static size_t MAX_SIZE = UnTech::Rooms::MAX_N_SCRIPTS;

        constexpr static auto SelectionPtr = &EditorT::scriptsSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.roomScripts.scripts; }
    };

    struct ScriptTriggers final : public Room {
        using ListT = std::vector<UnTech::Rooms::ScriptTrigger>;
        using ListArgsT = std::tuple<>;
        using SelectionT = MultipleSelection;

        constexpr static size_t MAX_SIZE = RM::MAX_SCRIPT_TRIGGERS;

        constexpr static auto SelectionPtr = &EditorT::scriptTriggersSel;

        static ListT* getList(EditorDataT& entityRomData) { return &entityRomData.scriptTriggers; }
    };

    struct ScriptStatements final : public Room {
        using ListT = std::vector<Scripting::ScriptNode>;
        using ListArgsT = std::tuple<NodeSelection::ParentIndexT>;
        using SelectionT = NodeSelection;
        using ParentActionPolicy = RoomEditorData::AP::Scripts;

        constexpr static size_t MAX_SIZE = NodeSelection::MAX_SIZE;

        constexpr static auto SelectionPtr = &EditorT::scriptStatementsSel;

        static ListT* getList(EditorDataT& data, const NodeSelection::ParentIndexT& parentIndex)
        {
            ListT* parent = nullptr;

            if (parentIndex.front() < data.roomScripts.scripts.size()) {
                parent = &data.roomScripts.scripts.at(parentIndex.front()).statements;
            }
            else {
                parent = &data.roomScripts.startupScript.statements;
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
        const unsigned targetLineNo;

        unsigned lineNo = 0;

        unsigned depth = 0;
        unsigned index = 0;
        std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> parentIndex;

        std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> foundParentIndex;
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
    , _invalidTilesCompileId(0)
    , _entityTextureWindowOpen(false)
    , _mtTilesetValid(false)
{
}

bool RoomEditorGui::setEditorData(AbstractEditorData* data)
{
    AbstractMetaTileEditorGui::setEditorData(data);
    return (_data = dynamic_cast<RoomEditorData*>(data));
}

void RoomEditorGui::resetState()
{
    AbstractMetaTileEditorGui::resetState();

    setEditMode(EditMode::SelectObjects);

    _invalidTilesCompileId = 0;

    _mtTilesetValid = false;
    _scenesData = nullptr;
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
            EditorFieldActions<AP::Scene>::fieldEdited(_data);
        }

        if (ImGui::InputUsize("Map Size", &_mapSize, AP::Map::MAX_SIZE)) {
            _mapSize.width = std::max(_mapSize.width, AP::Map::MIN_SIZE.width);
            _mapSize.height = std::max(_mapSize.height, AP::Map::MIN_SIZE.height);
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            GridActions<AP::Map>::resizeGrid(_data, _mapSize);
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

        for (auto [i, en] : enumerate(room.entrances)) {
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

    constexpr static std::array<const char8_t*, RM::MAX_ENTITY_GROUPS + 4> entityGroupNames{
        u8"Entity Group 0",
        u8"Entity Group 1",
        u8"Entity Group 2",
        u8"Entity Group 3",
        u8"Entity Group 4",
        u8"Entity Group 5",
        u8"Entity Group 6",
        u8"Entity Group 7",
        u8"Entity Group OUT OF BOUNDS",
        u8"Entity Group OUT OF BOUNDS",
        u8"Entity Group OUT OF BOUNDS",
        u8"Entity Group OUT OF BOUNDS",
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
        for (const auto groupIndex : range(nGroups)) {
            auto& group = room.entityGroups.at(groupIndex);

            if (ImGui::TreeNodeToggleSelection(u8Cast(entityGroupNames.at(groupIndex)), &_data->entityGroupsSel, groupIndex)) {
                {
                    bool edited = false;

                    ImGui::InputIdstring("Name", &group.name);
                    edited |= ImGui::IsItemDeactivatedAfterEdit();

                    if (edited) {
                        ListActions<AP::EntityGroups>::itemEdited(_data, groupIndex);
                    }
                }
                ImGui::Spacing();

                for (auto [i, entity] : enumerate(group.entities)) {
                    bool edited = false;

                    ImGui::PushID(i);

                    const std::u8string selLabel = stringBuilder(u8"Entity ", entityId);
                    entityId++;

                    ImGui::Selectable(u8Cast(selLabel), &_data->entityEntriesSel, groupIndex, i);

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

void RoomEditorGui::scriptTriggersWindow()
{
    assert(_data);
    auto& room = _data->data;

    ImGui::SetNextWindowSize(ImVec2(325, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Script Triggers##Room", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        ListButtons<AP::ScriptTriggers>(_data);

        ImGui::BeginChild("Scroll");

        ImGui::Columns(4);

        ImGui::Separator();
        ImGui::NextColumn();
        ImGui::Text("Script");
        ImGui::NextColumn();
        ImGui::Text("AABB");
        ImGui::NextColumn();
        ImGui::Text("Once");
        ImGui::NextColumn();
        ImGui::Separator();

        const usize bounds(room.mapRight(), room.mapBottom());

        for (auto [i, st] : enumerate(room.scriptTriggers)) {
            bool edited = false;

            ImGui::PushID(i);

            ImGui::Selectable(&_data->scriptTriggersSel, i);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::IdStringCombo("##script", &st.script, room.roomScripts.scripts);
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            ImGui::InputUrect("##aabb", &st.aabb, bounds);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            ImGui::NextColumn();

            ImGui::SetNextItemWidth(-1);
            edited |= ImGui::Checkbox("Once", &st.once);
            ImGui::NextColumn();

            if (edited) {
                ListActions<AP::ScriptTriggers>::itemEdited(_data, i);
            }

            ImGui::PopID();
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
        for (auto [i, eg] : const_enumerate(_entityGraphics->entities)) {
            if (filter.PassFilter(u8Cast(eg.name))) {
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

            unsigned entityIndex = *reinterpret_cast<const unsigned*>(payload->Data);

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
                ImGui::BeginTooltip();
                ImGui::Text("Extrance %u %s", unsigned(i), u8Cast(entrance.name));
                ImGui::EndTooltip();
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
                        ImGui::BeginTooltip();
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
                ImGui::BeginTooltip();
                if (!st.once) {
                    ImGui::Text("Script Trigger %u: %s", unsigned(i), u8Cast(st.script));
                }
                else {
                    ImGui::Text("Script Trigger %u: %s (once)", unsigned(i), u8Cast(st.script));
                }
                ImGui::EndTooltip();
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
            ImGui::TextUnformatted(u8"Selected Entity Group");
            ImGui::EndTooltip();
        }
        ImGui::SameLine(0.0f, 12.0f);

        ImGui::SetNextItemWidth(180);
        ImGui::SingleSelectionNamedListCombo("##PlayerId", &playerId, _entityGraphics->players, false);
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextUnformatted(u8"Player Entity");
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
        ImGui::ToggledButtonWithTooltip("S##showScriptTriggers", &showScriptTriggers, "Show Script Triggers");
        ImGui::SameLine();

        Style::roomEditorZoom.zoomCombo("##zoom");

        ImGui::BeginChild("Scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        const ImVec2& zoom = Style::roomEditorZoom.zoom();
        const rect bounds = room.validEntityArea();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        _graphics.setDisabled(editMode() != EditMode::SelectObjects);

        _graphics.startLoop("Room", bounds, zoom,
                            &_data->entrancesSel, &_data->entityEntriesSel, &_data->scriptTriggersSel);

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
                          &_data->entrancesSel, &_data->entityEntriesSel, &_data->scriptTriggersSel);

        _invalidTiles.draw(drawList, geo.zoom, geo.offset);

        // Draw drag+drop entity on top of selected entity outlines
        entityDropTarget(drawList);

        if (_graphics.isEditingFinished()) {
            _data->startMacro();

            ListActions<AP::Entrances>::selectedItemsEdited(_data);
            ListActions<AP::EntityEntries>::selectedItemsEdited(_data);
            ListActions<AP::ScriptTriggers>::selectedItemsEdited(_data);

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

    // Update mapSize if the map changed
    if (!_tilemapValid) {
        _mapSize = _data->data.map.size();
    }

    updateMapAndProcessAnimations();
    updateEntityGraphics();
    updateTilesetData(projectFile, projectData);
    updateInvalidTileList(projectData);

    propertiesWindow(projectFile);
    entrancesWindow();
    entitiesWindow();
    roomEntitiesWindow(projectFile);
    scriptTriggersWindow();

    editorWindow();

    tilesetMinimapWindow("Tileset##Room");

    minimapWindow("Minimap##Room");

    scriptsWindow(projectFile, projectData);

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
    ImGui::MenuItem("Show Script Triggers", nullptr, &showScriptTriggers);

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

    _tilesetShader.setPaletteData(projectData.palettes().at(paletteIndex));

    const auto mtData = projectData.metaTileTilesets().at(tilesetIndex);
    const bool mtDataChanged = mtData != _tilesetShader.tilesetData();

    if (mtDataChanged || !mtData) {
        auto tileset = tilesetIndex < projectFile.metaTileTilesets.size()
                           ? projectFile.metaTileTilesets.at(tilesetIndex)
                           : std::nullopt;

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

void RoomEditorGui::updateInvalidTileList(const Project::ProjectData& projectData)
{
    assert(_data);

    projectData.rooms().readResourceState(
        _data->itemIndex().index, [&](const Project::ResourceStatus& status) {
            if (status.compileId != _invalidTilesCompileId) {
                _invalidTilesCompileId = status.compileId;
                _invalidTiles.clear();

                for (const auto& errorItem : status.errorList.list()) {
                    if (auto* tileErr = dynamic_cast<const Rooms::InvalidRoomTilesError*>(errorItem.get())) {
                        _invalidTiles.append(*tileErr);
                    }
                }
            }
        });
}

// Room Scripts
// ============

class RoomScriptGuiVisitor {
    using AP = RoomEditorData::AP;

private:
    RoomEditorData* const data;
    const Scripting::BytecodeMapping& bcMapping;
    const Project::ProjectFile& projectFile;

    unsigned depth;

    std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> parentIndex;
    unsigned index;

    constexpr static std::array<const char*, 2> argLabels = { "##Arg1", "##Arg2" };

    bool openAddMenu;
    static std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> addMenuParentIndex;

    const ImVec4 disabledColor;

    constexpr static float INDENT_SPACING = 30;

public:
    RoomScriptGuiVisitor(RoomEditorData* d,
                         const Scripting::BytecodeMapping& mapping,
                         const Project::ProjectFile& projectFile)
        : data(d)
        , bcMapping(mapping)
        , projectFile(projectFile)
        , depth(0)
        , parentIndex()
        , index(0)
        , openAddMenu(false)
        , disabledColor(ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled))
    {
        assert(data != nullptr);
    }

    void processGui(Scripting::Script& script, const unsigned scriptId)
    {
        ImGui::PushItemWidth((ImGui::GetWindowWidth() - 30) / 4);

        processStatements_root(script.statements, scriptId);

        ImGui::PopItemWidth();

        processAddMenu();

        assert(depth == 0);
    }

    bool roomArgument(const char* label, std::u8string* value)
    {
        bool edited = false;

        if (ImGui::BeginCombo(label, u8Cast(*value))) {
            for (const auto& item : projectFile.rooms) {
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
            ImGui::BeginTooltip();
            ImGui::Text("Room");
            ImGui::EndTooltip();
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
            ImGui::BeginTooltip();
            ImGui::Text("Room Entrance");
            ImGui::EndTooltip();
        }

        return edited;
    }

    bool statementArgument(const char* label, const Scripting::ArgumentType& type, std::u8string* value)
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
            ImGui::InputText(label, value);
            edited = ImGui::IsItemDeactivatedAfterEdit();

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();

                switch (type) {
                case Type::Unused:
                    break;

                case Type::Flag:
                    ImGui::TextUnformatted(u8"Flag");
                    break;

                case Type::Word:
                    ImGui::TextUnformatted(u8"Word");
                    break;

                case Type::ImmediateU16:
                    ImGui::TextUnformatted(u8"Immediate U16");
                    break;

                case Type::RoomScript:
                    ImGui::TextUnformatted(u8"Room Script");
                    break;

                case Type::EntityGroup:
                    ImGui::TextUnformatted(u8"Entity Group");
                    break;

                case Type::Room:
                    ImGui::TextUnformatted(u8"Room");
                    break;

                case Type::RoomEntrance:
                    ImGui::TextUnformatted(u8"Room Entrance");
                    break;
                }

                ImGui::EndTooltip();
            }
        } break;

        case Type::Room: {
            edited = roomArgument(label, value);
        } break;

        case Type::RoomEntrance: {
            edited = roomEntranceArgument(label, value,
                                          [this]() { return &data->data; });
        } break;
        }

        return edited;
    }

    bool roomAndRoomEntraceArguments(std::array<std::u8string, 2>& arguments)
    {
        bool edited = false;

        ImGui::SameLine();
        edited |= roomArgument(argLabels.at(0), &arguments.at(0));

        ImGui::SameLine();
        edited |= roomEntranceArgument(argLabels.at(1), &arguments.at(1),
                                       [&]() { return projectFile.rooms.find(idstring::fromString(arguments.at(0))); });

        return edited;
    }

    bool scriptArguments(const Scripting::InstructionData& bc, std::array<std::u8string, 2>& arguments)
    {
        constexpr std::array<Scripting::ArgumentType, 2> loadRoomArgs = { Scripting::ArgumentType::Room, Scripting::ArgumentType::RoomEntrance };

        if (bc.arguments == loadRoomArgs) {
            return roomAndRoomEntraceArguments(arguments);
        }
        else {
            bool edited = false;

            assert(bc.arguments.size() == arguments.size());
            for (const auto i : range(arguments.size())) {
                edited |= statementArgument(argLabels.at(i), bc.arguments.at(i), &arguments.at(i));
            }

            return edited;
        }
    }

    void operator()(Scripting::Statement& statement)
    {
        bool edited = false;

        edited |= ImGui::IdStringCombo("##Name", &statement.opcode, bcMapping.instructionNames);

        auto it = bcMapping.instructions.find(statement.opcode);
        if (it != bcMapping.instructions.end()) {
            const auto& bc = it->second;

            edited |= scriptArguments(bc, statement.arguments);
        }

        if (edited) {
            ListActions<AP::ScriptStatements>::itemEdited(data, parentIndex, index);
        }
    }

    void operator()(Scripting::IfStatement& s)
    {
        ImGui::TextUnformatted(u8"if ");
        ImGui::SameLine();

        const bool edited = condition(&s.condition);
        if (edited) {
            ListActionsVariant<AP::ScriptStatements>::variantFieldEdited<&Scripting::IfStatement::condition>(data, parentIndex, index);
        }

        processChildStatements(s.thenStatements, s.elseStatements);
    }

    void operator()(Scripting::WhileStatement& s)
    {
        ImGui::TextUnformatted(u8"while ");
        ImGui::SameLine();

        const bool edited = condition(&s.condition);
        if (edited) {
            ListActionsVariant<AP::ScriptStatements>::variantFieldEdited<&Scripting::WhileStatement::condition>(data, parentIndex, index);
        }

        processChildStatements(s.statements);
    }

    void operator()(Scripting::Comment& comment)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, Style::commentColor());

        bool edited = false;

        ImGui::TextUnformatted(u8"//");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##Text", &comment.text);
        edited |= ImGui::IsItemDeactivatedAfterEdit();

        if (edited) {
            ListActions<AP::ScriptStatements>::itemEdited(data, parentIndex, index);
        }

        ImGui::PopStyleColor();
    }

private:
    bool condition(Scripting::Conditional* c)
    {
        bool edited = false;

        ImGui::SetNextItemWidth(75);
        edited |= ImGui::EnumCombo("##type", &c->type);
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
        ImGui::InputIdstring("##var", &c->variable);
        edited |= ImGui::IsItemDeactivatedAfterEdit();

        ImGui::SameLine();
        ImGui::SetNextItemWidth(75);
        edited |= ImGui::EnumCombo("##comp", &c->comparison, c->type);

        switch (c->type) {
        case Scripting::ConditionalType::Flag:
            break;

        case Scripting::ConditionalType::Word:
            ImGui::SameLine();
            ImGui::InputText("##value", &c->value);
            edited |= ImGui::IsItemDeactivatedAfterEdit();
            break;
        }

        return edited;
    }

    void openProcessMenu()
    {
        addMenuParentIndex = parentIndex;
        openAddMenu = true;
    }

    // ::TODO use menu to add statements anywhere::
    void processAddMenu()
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
                    ListActions<AP::ScriptStatements>::addItem(data, addMenuParentIndex, newStatement);
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
                        ListActions<AP::ScriptStatements>::addItem(data, addMenuParentIndex, newStatement);
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
                        ListActions<AP::ScriptStatements>::addItem(data, addMenuParentIndex, newStatement);
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
                ListActions<AP::ScriptStatements>::addItem(data, addMenuParentIndex, Scripting::Comment{});
            }

            ImGui::EndPopup();
        }
    }

    void processStatements__afterParentIndexUpdated(std::vector<Scripting::ScriptNode>& statements)
    {
        auto& sel = data->scriptStatementsSel;

        const bool isParentSelected = parentIndex == sel.parentIndex();

        // Save index - prevents an infinite loop
        const unsigned oldIndex = index;

        const float selSpacing = ImGui::GetCursorPosX() + INDENT_SPACING;

        for (auto [i, s] : enumerate(statements)) {
            index = i;

            ImGui::PushID(index);

            ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);
            const std::u8string label = stringBuilder(index);
            if (ImGui::Selectable(u8Cast(label), isParentSelected && index == sel.selectedIndex(), ImGuiSelectableFlags_AllowItemOverlap)) {
                sel.setSelected(parentIndex, index);
            }
            ImGui::PopStyleColor();

            ImGui::SameLine(selSpacing);
            std::visit(*this, s);

            ImGui::PopID();
        }

        // restore index
        index = oldIndex;
    }

    void processStatements_root(std::vector<Scripting::ScriptNode>& statements, const unsigned scriptId)
    {
        depth = 0;
        parentIndex.fill(0xffff);
        parentIndex.front() = scriptId;
        index = scriptId;

        processStatements__afterParentIndexUpdated(statements);

        if (ImGui::Button("Add")) {
            openProcessMenu();
        }
    }

    void processChildStatements(std::vector<Scripting::ScriptNode>& statements)
    {
        if (depth + 1 >= parentIndex.size()) {
            ImGui::TextUnformatted(u8"ERROR: MAXIMUM DEPTH REACHED");
            return;
        }
        depth++;
        parentIndex.at(depth) = index & 0x7fff;

        ImGui::Indent(INDENT_SPACING);

        processStatements__afterParentIndexUpdated(statements);

        if (ImGui::Button("Add")) {
            openProcessMenu();
        }

        ImGui::Unindent(INDENT_SPACING);

        parentIndex.at(depth) = NodeSelection::NO_SELECTION;
        depth--;
    }

    void processChildStatements(std::vector<Scripting::ScriptNode>& thenStatements, std::vector<Scripting::ScriptNode>& elseStatements)
    {
        if (depth + 1 >= parentIndex.size()) {
            ImGui::TextUnformatted(u8"ERROR: MAXIMUM DEPTH REACHED");
            return;
        }
        depth++;
        parentIndex.at(depth) = index & 0x7fff;

        ImGui::Indent(INDENT_SPACING);

        processStatements__afterParentIndexUpdated(thenStatements);

        if (ImGui::Button("Add")) {
            openProcessMenu();
        }

        // set else flag in parentIndex.
        parentIndex.at(depth) |= 0x8000;

        if (!elseStatements.empty()) {
            ImGui::TextUnformatted(u8"else");

            processStatements__afterParentIndexUpdated(elseStatements);

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
};

std::array<uint16_t, Scripting::Script::MAX_DEPTH + 1> RoomScriptGuiVisitor::addMenuParentIndex;

template <typename AP>
static void tempVariableList(RoomEditorData* data)
{
    assert(data);
    std::vector<idstring>* list = AP::getList(data->data);
    assert(list);

    ListButtons<AP>(data);

    ImGui::Separator();

    auto& sel = data->*AP::SelectionPtr;

    for (auto [i, var] : enumerate(*list)) {
        bool edited = false;

        ImGui::PushID(i);

        ImGui::Selectable("##Sel", &sel, i, ImGuiSelectableFlags_AllowItemOverlap);
        ImGui::SameLine(30);

        ImGui::SetNextItemWidth(-1);
        ImGui::InputIdstring("##Name", &var);
        edited |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::NextColumn();

        if (edited) {
            ListActions<AP>::itemEdited(data, i);
        }

        ImGui::PopID();
    }

    ImGui::Separator();
}

void RoomEditorGui::scriptsWindow(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData)
{
    assert(_data);
    auto& roomScripts = _data->data.roomScripts;

    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Room Scripts", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {

        ImGui::BeginChild("Temp-Vars", ImVec2(200, 0), true);
        {
            ImGui::PushID("Flags");

            ImGui::TextUnformatted(u8"Temporary Flags:\n"
                                   "(cleared on room load)");
            tempVariableList<AP::TempScriptFlags>(_data);

            ImGui::PopID();

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::PushID("Words");

            ImGui::TextUnformatted(u8"Temporary Words:\n"
                                   "(reset to 0 on room load)");
            tempVariableList<AP::TempScriptWords>(_data);

            ImGui::PopID();
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

        ImGui::BeginGroup();
        ImGui::BeginChild("Script");

        const auto bcMapping = projectData.bytecodeData();
        if (bcMapping) {
            const bool isStartupScript = _data->scriptsSel.selectedIndex() >= roomScripts.scripts.size();
            const unsigned scriptId = !isStartupScript ? _data->scriptsSel.selectedIndex() : NodeSelection::NO_SELECTION;
            Scripting::Script& script = !isStartupScript ? roomScripts.scripts.at(scriptId) : roomScripts.startupScript;

            if (!isStartupScript) {
                ImGui::InputIdstring("Name", &script.name);
                if (ImGui::IsItemDeactivatedAfterEdit()) {
                    // NOTE: Cannot edit the startup script using the AP::Scripts Action Policy.
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

            RoomScriptGuiVisitor sgVisitor(_data, *bcMapping, projectFile);
            sgVisitor.processGui(script, scriptId);

            ImGui::EndChild();
        }
        else {
            ImGui::TextUnformatted(u8"\n\n"
                                   "ERROR: Cannot view script: Missing bytecode data.");
        }

        ImGui::EndChild();
        ImGui::EndGroup();
    }
    ImGui::End();
}

}
