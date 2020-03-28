/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include <memory>

class QGraphicsScene;

namespace UnTech {
struct ErrorListItem;

namespace GuiQt {
class ZoomSettings;
class ZoomSettingsManager;
class ZoomableGraphicsView;
class PropertyListView;

namespace MetaTiles {
class Style;

namespace MtTileset {
namespace Ui {
class EditorWidget;
}
class MtTilesetPropertyManager;
class ResourceItem;
class MtTilesetRenderer;
class MtTilesetGraphicsScene;
class MtEditableScratchpadGraphicsScene;
class MtScratchpadGraphicsScene;
class TilePropertiesWidget;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

    enum StackIndex : int {
        ANIMATION_FRAMES_INPUT_WIDGET = 0,
        METATILES_CENTRAL_WIDGET = 1,
    };

    enum TabIndex : int {
        METATILES_TAB = 0,
        SCRATCHPAD_TAB = 1,
    };

public:
    explicit EditorWidget(ZoomSettingsManager* zoomSettingsManager, QWidget* parent = nullptr);
    ~EditorWidget();

    virtual QString windowStateName() const final;

    virtual ZoomSettings* zoomSettings() const final;
    virtual void populateMenu(QMenu* editMenu, QMenu* viewMenu) final;

    virtual bool setResourceItem(AbstractResourceItem* abstractItem) final;

    virtual void onErrorDoubleClicked(const ErrorListItem& error) final;

private slots:
    void onTilesetStateChanged();

    void onTilesetPalettesChanged();
    void onPaletteComboActivated(const QString& paletteId);

    void onTabChanged();

private:
    std::unique_ptr<Ui::EditorWidget> const _ui;

    // Used by the DockWidgets
    PropertyListView* const _propertyListView;
    TilePropertiesWidget* const _tilePropertiesWidget;
    ZoomableGraphicsView* const _dockedTilesetView;
    ZoomableGraphicsView* const _dockedScratchpadView;

    Style* const _style;
    MtTilesetPropertyManager* const _tilesetPropertyManager;
    MtTilesetRenderer* const _renderer;
    MtTilesetGraphicsScene* const _tilesetScene;
    MtScratchpadGraphicsScene* const _scratchpadScene;
    MtEditableScratchpadGraphicsScene* const _editableScratchpadScene;

    ResourceItem* _tileset;
};
}
}
}
}
