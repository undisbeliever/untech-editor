/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractmainwindow.h"
#include <QComboBox>
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

class MainWindow : public AbstractMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    void setupMenubar();
    void setupStatusbar();

protected:
    virtual void documentChangedEvent(AbstractDocument* document,
                                      AbstractDocument* oldDocument) final;
    virtual std::unique_ptr<AbstractDocument> createDocumentInstance() final;

private slots:
    void onSelectedFrameChanged();

private:
    Actions* _actions;
    ZoomSettings* _zoomSettings;
    LayerSettings* _layerSettings;

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
