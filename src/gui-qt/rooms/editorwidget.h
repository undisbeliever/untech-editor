/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
class PropertyListView;
class ZoomSettingsManager;
class ZoomableGraphicsView;

namespace MetaTiles {
class Style;
}
namespace MetaTiles::MtTileset {
class MtTilesetRenderer;
class MtTilesetGraphicsScene;
class MtScratchpadGraphicsScene;
}
namespace Entity::EntityRomEntries {
class EntitiesWithIconsModel;
}

namespace Rooms {
namespace Ui {
class EditorWidget;
}
class ResourceItem;
class RoomPropertyManager;
class RoomGraphicsScene;
class EditableRoomGraphicsScene;
class RoomEntitiesDock;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    explicit EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent = nullptr);
    ~EditorWidget();

    virtual QString windowStateName() const final;
    virtual ZoomSettings* zoomSettings() const final;
    virtual void populateMenu(QMenu* editMenu, QMenu* viewMenu) final;

    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

private slots:
    void updateTilesetAndPalette();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;

    ZoomableGraphicsView* const _dockedMinimapView;
    ZoomableGraphicsView* const _dockedTilesetView;
    ZoomableGraphicsView* const _dockedScratchpadView;

    MetaTiles::Style* const _style;
    MetaTiles::MtTileset::MtTilesetRenderer* const _renderer;

    EditableRoomGraphicsScene* const _editableRoomScene;
    RoomGraphicsScene* const _minimapRoomScene;
    MetaTiles::MtTileset::MtTilesetGraphicsScene* const _tilesetScene;
    MetaTiles::MtTileset::MtScratchpadGraphicsScene* const _scratchpadScene;

    RoomEntitiesDock* const _roomEntitiesDock;

    RoomPropertyManager* const _propertyManager;
    Entity::EntityRomEntries::EntitiesWithIconsModel* const _entitiesWithIconsModel;

    ResourceItem* _resourceItem;
};

}
}
}
