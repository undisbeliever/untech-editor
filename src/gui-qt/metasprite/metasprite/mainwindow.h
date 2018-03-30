/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QComboBox>
#include <QMainWindow>
#include <QPushButton>
#include <QTabWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
class ZoomSettings;
class ZoomableGraphicsView;

namespace MetaSprite {
class LayerSettings;

namespace Animation {
class AnimationDock;
class AnimationPreview;
}
namespace MetaSprite {

class Document;
class Actions;
class FrameListModel;
class TilesetPixmaps;
class MsGraphicsScene;
class MsAnimationPreviewItemFactory;
class FrameSetDock;
class FrameDock;
class PalettesDock;
class TilesetDock;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(ZoomSettings* zoomSettings, QWidget* parent = nullptr);
    ~MainWindow();

    QPushButton* layersButton() { return _layersButton; }

    void setDocument(Document* document);

    void populateMenu(QMenu* editMenu, QMenu* viewMenu);

private slots:
    void onSelectedFrameChanged();

private:
    Document* _document;

    FrameListModel* const _frameListModel;
    Actions* const _actions;
    LayerSettings* const _layerSettings;

    QPushButton* const _layersButton;

    TilesetPixmaps* const _tilesetPixmaps;

    QTabWidget* const _tabWidget;
    ZoomableGraphicsView* const _graphicsView;
    MsGraphicsScene* const _graphicsScene;
    Animation::AnimationPreview* const _animationPreview;

    MsAnimationPreviewItemFactory* const _animationPreviewItemFactory;

    FrameSetDock* const _frameSetDock;
    FrameDock* const _frameDock;
    Animation::AnimationDock* const _animationDock;
    PalettesDock* const _palettesDock;
    TilesetDock* const _tilesetDock;
};
}
}
}
}
