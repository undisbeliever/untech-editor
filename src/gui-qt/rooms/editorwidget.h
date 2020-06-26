/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include <QListView>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettingsManager;
class OpenGLZoomableGraphicsView;

namespace Accessor {
class ListAccessorTableDock;
}
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
class RoomEntranceManager;
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

    virtual void onErrorDoubleClicked(const ErrorListItem& error) final;

private slots:
    void updateTilesetAndPalette();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;

    OpenGLZoomableGraphicsView* const _dockedMinimapView;
    OpenGLZoomableGraphicsView* const _dockedTilesetView;
    OpenGLZoomableGraphicsView* const _dockedScratchpadView;

    MetaTiles::Style* const _style;
    MetaTiles::MtTileset::MtTilesetRenderer* const _renderer;

    EditableRoomGraphicsScene* const _editableRoomScene;
    RoomGraphicsScene* const _minimapRoomScene;
    MetaTiles::MtTileset::MtTilesetGraphicsScene* const _tilesetScene;
    MetaTiles::MtTileset::MtScratchpadGraphicsScene* const _scratchpadScene;

    RoomPropertyManager* const _propertyManager;
    RoomEntranceManager* const _roomEntranceManager;
    Entity::EntityRomEntries::EntitiesWithIconsModel* const _entitiesWithIconsModel;

    QListView* const _entitiesListView;

    QDockWidget* const _propertyDock;
    QDockWidget* const _minimapDock;
    QDockWidget* const _tilesetDock;
    QDockWidget* const _scratchpadDock;
    QDockWidget* const _entitiesListDock;
    RoomEntitiesDock* const _roomEntitiesDock;
    Accessor::ListAccessorTableDock* const _roomEntrancesDock;

    ResourceItem* _resourceItem;
};

}
}
}
