/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstracteditorwidget.h"
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

class Document;
class TilesetPixmaps;
class MsGraphicsScene;
class MsAnimationPreviewItemFactory;
class FrameSetDock;
class FrameDock;
class PalettesDock;
class TilesetDock;

class EditorWidget : public AbstractEditorWidget {
    Q_OBJECT

public:
    explicit EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent = nullptr);
    ~EditorWidget();

    virtual QList<QDockWidget*> createDockWidgets(QMainWindow* mainWindow) final;

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
    Document* _document;

    LayerSettings* const _layerSettings;
    QPushButton* const _layersButton;

    TilesetPixmaps* const _tilesetPixmaps;

    FrameSetDock* const _frameSetDock;
    FrameDock* const _frameDock;
    Animation::AnimationDock* const _animationDock;
    PalettesDock* const _palettesDock;
    TilesetDock* const _tilesetDock;

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
