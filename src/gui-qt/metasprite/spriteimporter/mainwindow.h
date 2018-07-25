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
class ZoomSettingsManager;
class ZoomableGraphicsView;

namespace MetaSprite {
class LayerSettings;

namespace Animation {
class AnimationDock;
class AnimationPreview;
}
namespace SpriteImporter {

class Document;
class TilesetPixmaps;
class SiGraphicsScene;
class SiAnimationPreviewItemFactory;
class FrameSetDock;
class FrameDock;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(ZoomSettingsManager* zoomManager, QWidget* parent = nullptr);
    ~MainWindow();

    QPushButton* layersButton() { return _layersButton; }
    ZoomSettings* zoomSettings() const;

    void populateMenu(QMenu* editMenu, QMenu* viewMenu);

    void setDocument(Document* document);

signals:
    void currentTabChanged();

private slots:
    void populateWidgets();

    void onSelectedFrameChanged();

private:
    Document* _document;

    LayerSettings* const _layerSettings;

    QPushButton* const _layersButton;

    FrameSetDock* const _frameSetDock;
    FrameDock* const _frameDock;
    Animation::AnimationDock* const _animationDock;

    QTabWidget* const _tabWidget;
    ZoomableGraphicsView* const _graphicsView;
    SiGraphicsScene* const _graphicsScene;
    Animation::AnimationPreview* const _animationPreview;

    SiAnimationPreviewItemFactory* const _animationPreviewItemFactory;
};
}
}
}
}
