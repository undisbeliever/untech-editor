/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

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
    friend class RoomScriptGuiVisitor;
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

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void saveFile() const final;
    virtual void errorDoubleClicked(const AbstractError*) final;
    virtual void updateSelection() final;

protected:
    virtual grid<uint8_t>& map() final;
    virtual void mapTilesPlaced(const urect r) final;

    virtual void selectedTilesetTilesChanged() final;
    virtual void selectedTilesChanged() final;

    void selectedScratchpadTilesChanged();
    void clearSelectedTiles();
};

class RoomEditorGui final : public AbstractMetaTileEditorGui {
private:
    using AP = RoomEditorData::AP;

    RoomEditorData* _data;

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
    SplitterBarState _minimapSidebar;
    SplitterBarState _minimapBottombar;

    ImVec2 _entitiesDropdownWindowPos;
    bool _showEntitiesDropdownWindow;

    bool _entityTextureWindowOpen;

public:
    bool _mtTilesetValid;

    static unsigned playerId;

    static bool showEntrances;
    static bool showEntities;
    static bool showScriptTriggers;

public:
    RoomEditorGui();

    virtual bool setEditorData(AbstractEditorData* data) final;
    virtual void resetState() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile,
                            const Project::ProjectData& projectData) final;

    virtual void processExtraWindows(const Project::ProjectFile& projectFile,
                                     const Project::ProjectData& projectData) final;

    virtual void viewMenu() final;

protected:
    virtual void selectionChanged() final;

    virtual const std::array<idstring, 256>& tileFunctionTables() const final;

private:
    void propertiesGui(const Project::ProjectFile& projectFile);
    void scratchpadGui();

    void editorGui();
    void scriptsGui(const Project::ProjectFile& projectFile, const Project::ProjectData& projectData);

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
