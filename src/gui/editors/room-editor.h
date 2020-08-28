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

class AabbGraphics;
class EntityGraphics;

class RoomEditor final : public AbstractMetaTileEditor {
private:
    struct AP;

    UnTech::Rooms::RoomInput _data;
    usize _mapSize;

    MultipleSelection _entrancesSel;
    SingleSelection _entityGroupsSel;
    GroupMultipleSelection _entityEntriesSel;

    upoint_vectorset _selectedScratchpadTiles;

    static bool _tilesetAndPaletteIndexValid;

    static AabbGraphics _graphics;
    static std::shared_ptr<const EntityGraphics> _entityGraphics;

public:
    RoomEditor(ItemIndex itemIndex);

    virtual bool loadDataFromProject(const Project::ProjectFile& projectFile) final;

    virtual void editorOpened() final;
    virtual void editorClosed() final;

    virtual void processGui(const Project::ProjectFile& projectFile) final;
    virtual void updateSelection() final;

protected:
    virtual grid<uint8_t>& map() final;
    virtual void mapTilesPlaced(const urect r) final;

    virtual void selectedTilesetTilesChanged() final;
    virtual void selectedTilesChanged() final;
    void selectedScratchpadTilesChanged();
    void clearSelectedTiles();

private:
    void propertiesWindow(const Project::ProjectFile& projectFile);
    void entrancesWindow();
    void roomEntitiesWindow(const Project::ProjectFile& projectFile);

    void editorWindow();

    void drawObjects(ImDrawList* drawList) const;
    void drawAndEditObjects(ImDrawList* drawList);

    const grid<uint8_t>& scratchpad(const Project::ProjectFile& projectFile) const;

    void updateEntityGraphics();
    void updateTilesetAndPaletteIndex(const Project::ProjectFile& projectFile);

    static Texture& entityTexture();
};

}
