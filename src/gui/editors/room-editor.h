/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "room-script-gui.h"
#include "gui/editors/abstract-metatile-editor.h"
#include "gui/graphics/aabb-graphics.h"
#include "gui/graphics/invalid-room-tile-graphics.h"
#include "gui/selection.h"
#include "gui/splitter.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class AabbGraphics;
struct EntityGraphics;

class RoomEditorData final : public AbstractMetaTileEditorData {
private:
    friend class RoomEditorGui;
    friend class RoomScriptGui;
    struct AP;

public:
    UnTech::Rooms::RoomInput data;

private:
    MultipleSelection entrancesSel;
    SingleSelection entityGroupsSel;
    GroupMultipleSelection entityEntriesSel;
    MultipleSelection scriptTriggersSel;

    MultipleSelection tempScriptFlagsSel;
    MultipleSelection tempScriptWordsSel;

    SingleSelection scriptsSel;
    NodeSelection scriptStatementsSel;

    upoint_vectorset selectedScratchpadTiles;

public:
    explicit RoomEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void saveFile() const final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;

protected:
    void selectedScratchpadTilesChanged();
    void clearSelectedTiles();
};

class RoomEditorGui final : public AbstractMetaTileEditorGui {
private:
    using AP = RoomEditorData::AP;

    std::shared_ptr<RoomEditorData> _data;

    RoomScriptGui _roomScriptGui;

    Shaders::MtTilemap _scratchpadTilemap;

    std::array<idstring, 256> _tileFunctionTables;

    usize _mapSize;

    AabbGraphics _graphics;
    Texture _entityTexture;
    std::shared_ptr<const EntityGraphics> _entityGraphics;

    // Used to determine if the compiled scenes data has changed.
    std::shared_ptr<const Resources::CompiledScenesData> _scenesData;

    grid<uint8_t> _scratchpad;

    InvalidRoomTileGraphics _invalidTiles;
    unsigned _invalidTilesCompileId;

    SplitterBarState _sidebar;
    SplitterBarState _minimapRight_sidebar;
    SplitterBarState _minimapRight_bottombar;
    SplitterBarState _minimapBottom_sidebar;
    SplitterBarState _minimapBottom_rightbar;

    ImVec2 _entitiesDropdownWindowPos;
    bool _showEntitiesDropdownWindow;

    // If false, the minimap is on the bottom
    bool _minimapOnRight;

    bool _entityTextureWindowOpen;

public:
    bool _mtTilesetValid;

    static unsigned playerId;

    static bool showEntrances;
    static bool showEntities;
    static bool showScriptTriggers;

public:
    RoomEditorGui();

    bool setEditorData(const std::shared_ptr<AbstractEditorData>& data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

    void processExtraWindows(const Project::ProjectFile& projectFile,
                             const Project::ProjectData& projectData) final;

    void viewMenu() final;

protected:
    [[nodiscard]] grid<uint8_t>& map() final;
    void mapTilesPlaced(const urect r) final;

    void selectedTilesetTilesChanged() final;
    void selectedTilesChanged() final;

    void selectionChanged() final;

    [[nodiscard]] const std::array<idstring, 256>& tileFunctionTables() const final;

private:
    void propertiesGui(const Project::ProjectFile& projectFile);
    void scratchpadGui();

    void editorGui();

    void entityTextureWindow();

    void entitiesDropdownWindow();
    void entityDropTarget(ImDrawList* drawList);

    void drawObjects(ImDrawList* drawList);
    void drawAndEditObjects(ImDrawList* drawList);

    void updateEntityGraphics();
    void updateTilesetData(const Project::ProjectFile& projectFile,
                           const Project::ProjectData& projectData);
    void updateInvalidTileList(const Project::ProjectData& projectData);
};

}
