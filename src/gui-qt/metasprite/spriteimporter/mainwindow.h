/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QComboBox>
#include <QFileSystemWatcher>
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
namespace SpriteImporter {

class Document;
class Actions;
class TilesetPixmaps;
class SiGraphicsScene;
class SiAnimationPreviewItemFactory;
class FrameSetDock;
class FrameDock;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(ZoomSettings* zoomSettings, QWidget* parent = nullptr);
    ~MainWindow();

    QPushButton* layersButton() { return _layersButton; }

    void setDocument(Document* document);

    void populateMenu(QMenu* editMenu, QMenu* viewMenu);

private slots:
    void onSelectedFrameChanged();

    void onFrameSetImageFilenameChanged();
    void onImageFileChanged();

private:
    Document* _document;

    QFileSystemWatcher _imageFileWatcher;

    Actions* const _actions;
    LayerSettings* const _layerSettings;

    QPushButton* const _layersButton;

    QTabWidget* const _tabWidget;
    ZoomableGraphicsView* const _graphicsView;
    SiGraphicsScene* const _graphicsScene;
    Animation::AnimationPreview* const _animationPreview;

    SiAnimationPreviewItemFactory* const _animationPreviewItemFactory;

    FrameSetDock* const _frameSetDock;
    FrameDock* const _frameDock;
    Animation::AnimationDock* const _animationDock;
};
}
}
}
}
