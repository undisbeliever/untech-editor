/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui/editors/abstract-metatile-editor.h"
#include "gui/selection.h"
#include "models/project/project.h"

namespace UnTech::Gui {

class MetaTileTilesetEditor final : public AbstractMetaTileEditor {
private:
    struct TileProperties {
        MetaTiles::TileCollisionType tileCollision;
        bool tileCollisionSame;

        std::array<bool, 4> tilePriorities;
        std::array<bool, 4> tilePrioritiesSame;

        // Interactive Tiles Function Table to use
        idstring functionTable;
        bool functionTableSame;
    };

private:
    struct AP;

    UnTech::MetaTiles::MetaTileTilesetInput _data;
    usize _scratchpadSize;

    // safe - only one editor open at any given time
    static std::optional<TileProperties> _tileProperties;

public:
    MetaTileTilesetEditor(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;
    virtual void saveFile() const final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;

protected:
    virtual grid<uint8_t>& map() final;
    virtual void mapTilesPlaced(const urect r) final;

    virtual void selectedTilesetTilesChanged() final;
    virtual void selectedTilesChanged() final;

private:
    void resetTileProperties();
    void updateTileProperties();
    void tileCollisionClicked(const MetaTiles::TileCollisionType tct);
    void tilePriorityClicked(const unsigned subTile, const bool v);
    void tileFunctionTableSelected(const idstring& ft);

    void propertiesWindow(const Project::ProjectFile& projectFile);
    void tilePropertiesWindow(const Project::ProjectFile& projectFile);

    void tilesetWindow();
    void scratchpadWindow();
};

}
