/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/editors/abstract-metatile-editor.h"
#include "gui/graphics/invalid-image-error-graphics.h"
#include "gui/selection.h"
#include "gui/splitter.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class MetaTileTilesetEditorData final : public AbstractMetaTileEditorData {
private:
    friend class MetaTileTilesetEditorGui;
    struct AP;

    UnTech::MetaTiles::MetaTileTilesetInput data;

    SingleSelection tilesetFrameSel;
    SingleSelection paletteSel;

    // This allows `errorDoubleClicked` to invalidate MetaTileTilesetEditorGui::_tileProperties
    // without adding AbstractMetaTileEditorGui to `errorDoubleClicked`.
    bool tilePropertiesWindowValid{};

public:
    explicit MetaTileTilesetEditorData(ItemIndex itemIndex);

    bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    void saveFile() const final;
    void errorDoubleClicked(const AbstractError*) final;
    void updateSelection() final;

protected:
    grid<uint8_t>& map() final;
    void mapTilesPlaced(const urect r) final;

    void selectedTilesetTilesChanged() final;
    void selectedTilesChanged() final;
};

class MetaTileTilesetEditorGui final : public AbstractMetaTileEditorGui {
private:
    using AP = MetaTileTilesetEditorData::AP;

    struct TileProperties {
        MetaTiles::TileCollisionType tileCollision;
        bool tileCollisionSame;

        std::array<bool, 4> tilePriorities;
        std::array<bool, 4> tilePrioritiesSame;

        // Interactive Tiles Function Table to use
        idstring functionTable;
        bool functionTableSame;
    };

    MetaTileTilesetEditorData* _data;

    usize _scratchpadSize;
    std::optional<TileProperties> _tileProperties;

    InvalidImageErrorGraphics _invalidTilesCommon;
    std::vector<InvalidImageErrorGraphics> _invalidTilesFrame;
    unsigned _invalidTilesCompileId;

    SplitterBarState _sidebar;
    SplitterBarState _minimapRight_sidebar;
    SplitterBarState _minimapBottom_sidebar;

    // If false, the minimap is on the bottom
    bool _minimapOnRight;

public:
    bool _tilesetShaderImageFilenamesValid;
    bool _tileCollisionsValid;
    bool _interactiveTilesValid;

public:
    MetaTileTilesetEditorGui();

    bool setEditorData(AbstractEditorData* data) final;
    void resetState() final;
    void editorClosed() final;

    void processGui(const Project::ProjectFile& projectFile,
                    const Project::ProjectData& projectData) final;

    void viewMenu() final;

protected:
    void selectionChanged() final;
    [[nodiscard]] const std::array<idstring, 256>& tileFunctionTables() const final;

private:
    void resetTileProperties();
    void updateTileProperties();
    void tileCollisionClicked(const MetaTiles::TileCollisionType tct);
    void tilePriorityClicked(const unsigned subTile, const bool v);
    void tileFunctionTableSelected(const idstring& ft);

    void propertiesGui(const Project::ProjectFile& projectFile);
    void tilePropertiesGui(const Project::ProjectFile& projectFile);

    void tilesetGui();
    void scratchpadGui();

    void updateMtTilesetShader(const Project::ProjectFile& projectFile,
                               const Project::ProjectData& projectData);
    void updateInvalidTileList(const Project::ProjectData& projectData);
};

}
