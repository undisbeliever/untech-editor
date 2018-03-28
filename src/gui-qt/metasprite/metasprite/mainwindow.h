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

    Actions* _actions;
    LayerSettings* _layerSettings;

    QPushButton* _layersButton;

    TilesetPixmaps* _tilesetPixmaps;
    MsAnimationPreviewItemFactory* _animationPreviewItemFactory;

    QTabWidget* _tabWidget;
    ZoomableGraphicsView* _graphicsView;
    MsGraphicsScene* _graphicsScene;
    Animation::AnimationPreview* _animationPreview;

    FrameSetDock* _frameSetDock;
    FrameDock* _frameDock;
    Animation::AnimationDock* _animationDock;
    PalettesDock* _palettesDock;
    TilesetDock* _tilesetDock;

    QComboBox* _zoomComboBox;
    QComboBox* _aspectRatioComboBox;
};
}
}
}
}
