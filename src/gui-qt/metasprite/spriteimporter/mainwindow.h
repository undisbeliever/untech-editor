/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QComboBox>
#include <QFileSystemWatcher>
#include <QMainWindow>
#include <QTabWidget>
#include <QUndoGroup>
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

    void setDocument(Document* document);

    void setupMenubar(QMenu* editMenu, QMenu* viewMenu);
    void setupStatusbar(QStatusBar* statusBar);

private slots:
    void onSelectedFrameChanged();

    void onFrameSetImageFilenameChanged();
    void onImageFileChanged();

private:
    Document* _document;

    Actions* _actions;
    LayerSettings* _layerSettings;

    QFileSystemWatcher _imageFileWatcher;

    SiAnimationPreviewItemFactory* _animationPreviewItemFactory;

    QTabWidget* _tabWidget;
    ZoomableGraphicsView* _graphicsView;
    SiGraphicsScene* _graphicsScene;
    Animation::AnimationPreview* _animationPreview;

    FrameSetDock* _frameSetDock;
    FrameDock* _frameDock;
    Animation::AnimationDock* _animationDock;
};
}
}
}
}
