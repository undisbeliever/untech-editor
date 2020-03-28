/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
#include "gui-qt/accessor/accessor.h"
#include <QComboBox>
#include <QPushButton>
#include <QTabWidget>
#include <memory>

namespace UnTech {
struct ErrorListItem;

namespace GuiQt {
class ZoomSettings;
class ZoomSettingsManager;
class ZoomableGraphicsView;

namespace MetaSprite {
class LayerSettings;

namespace Animation {
class AnimationDock;
class AnimationPreview;
}
namespace MetaSprite {

class ResourceItem;
class TilesetPixmaps;
class MsGraphicsScene;
class MsAnimationPreviewItemFactory;
class FrameSetManager;
class FrameManager;
class FrameObjectManager;
class ActionPointManager;
class EntityHitboxManager;
class Actions;
class PalettesDock;
class TilesetDock;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    explicit EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent = nullptr);
    ~EditorWidget();

    virtual QString windowStateName() const final;

    virtual QPushButton* statusBarWidget() const final;
    virtual ZoomSettings* zoomSettings() const final;
    virtual void populateMenu(QMenu* editMenu, QMenu* viewMenu) final;

    virtual bool setResourceItem(AbstractResourceItem* item) final;

    virtual void onErrorDoubleClicked(const ErrorListItem& error) final;

private slots:
    void populateWidgets();

    void onSelectedFrameChanged();

private:
    void showGraphicsTab();

private:
    ResourceItem* _resourceItem;
    LayerSettings* const _layerSettings;
    QPushButton* const _layersButton;

    TilesetPixmaps* const _tilesetPixmaps;

    // Used by the docks
    FrameSetManager* const _frameSetManager;
    FrameManager* const _frameManager;
    FrameObjectManager* const _frameObjectManager;
    ActionPointManager* const _actionPointManager;
    EntityHitboxManager* const _entityHitboxManager;

    Accessor::NamedListDock* const _frameListDock;
    QDockWidget* const _frameSetDock;
    QDockWidget* const _framePropertiesDock;
    Accessor::MultipleSelectionTableDock* const _frameContentsDock;
    Animation::AnimationDock* const _animationDock;
    PalettesDock* const _palettesDock;
    TilesetDock* const _tilesetDock;

    Actions* const _actions;

    QTabWidget* const _tabWidget;
    ZoomableGraphicsView* const _graphicsView;
    MsGraphicsScene* const _graphicsScene;
    Animation::AnimationPreview* const _animationPreview;

    MsAnimationPreviewItemFactory* const _animationPreviewItemFactory;
};
}
}
}
}
