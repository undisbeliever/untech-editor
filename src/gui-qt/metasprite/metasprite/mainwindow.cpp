/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "accessors.h"
#include "actions.h"
#include "document.h"
#include "framedock.h"
#include "framelistmodel.h"
#include "framesetdock.h"
#include "msanimationpreviewitem.h"
#include "msgraphicsscene.h"
#include "palettesdock.h"
#include "tilesetdock.h"
#include "tilesetpixmaps.h"
#include "gui-qt/common/graphics/zoomablegraphicsview.h"
#include "gui-qt/common/graphics/zoomsettings.h"
#include "gui-qt/metasprite/animation/animationdock.h"
#include "gui-qt/metasprite/animation/animationpreview.h"
#include "gui-qt/metasprite/layersettings.h"

#include <QPushButton>
#include <QStatusBar>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

MainWindow::MainWindow(ZoomSettings* zoomSettings, QWidget* parent)
    : QMainWindow(parent)
    , _document(nullptr)
    , _frameListModel(new FrameListModel(this))
    , _actions(new Actions(this))
    , _layerSettings(new LayerSettings(this))
    , _layersButton(new QPushButton(tr("Layers"), this))
    , _tilesetPixmaps(new TilesetPixmaps(this))
    , _frameSetDock(new FrameSetDock(_frameListModel, _actions, this))
    , _frameDock(new FrameDock(_frameListModel, _actions, this))
    , _animationDock(new Animation::AnimationDock(this))
    , _palettesDock(new PalettesDock(_actions, this))
    , _tilesetDock(new TilesetDock(_tilesetPixmaps, this))
    , _tabWidget(new QTabWidget(this))
    , _graphicsView(new ZoomableGraphicsView(this))
    , _graphicsScene(new MsGraphicsScene(_actions, _layerSettings, _tilesetPixmaps, this))
    , _animationPreview(new Animation::AnimationPreview(_animationDock, this))
    , _animationPreviewItemFactory(new MsAnimationPreviewItemFactory(_layerSettings, _tilesetPixmaps, this))
{
    // Have the left and right docks take up the whole height of the documentWindow
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    QMenu* layerMenu = new QMenu(this);
    _layerSettings->populateMenu(layerMenu);
    _layersButton->setMenu(layerMenu);

    setCentralWidget(_tabWidget);
    _tabWidget->setTabPosition(QTabWidget::West);

    _graphicsView->setMinimumSize(256, 256);
    _graphicsView->setZoomSettings(zoomSettings);
    _graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);
    _graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    _graphicsView->setScene(_graphicsScene);
    _tabWidget->addTab(_graphicsView, tr("Frame"));

    _animationPreview->setZoomSettings(zoomSettings);
    _animationPreview->setItemFactory(_animationPreviewItemFactory);
    _tabWidget->addTab(_animationPreview, tr("Animation Preview"));

    addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);
    addDockWidget(Qt::RightDockWidgetArea, _frameDock);
    addDockWidget(Qt::RightDockWidgetArea, _animationDock);
    addDockWidget(Qt::BottomDockWidgetArea, _palettesDock);
    addDockWidget(Qt::BottomDockWidgetArea, _tilesetDock);

    tabifyDockWidget(_frameSetDock, _frameDock);
    tabifyDockWidget(_frameSetDock, _animationDock);

    resizeDocks({ _palettesDock }, { 1 }, Qt::Vertical);

    setDocument(nullptr);

    _frameSetDock->raise();
}

MainWindow::~MainWindow() = default;

void MainWindow::populateMenu(QMenu* editMenu, QMenu* viewMenu)
{
    editMenu->addSeparator();
    editMenu->addAction(_actions->raiseSelected());
    editMenu->addAction(_actions->lowerSelected());
    editMenu->addAction(_actions->cloneSelected());
    editMenu->addAction(_actions->removeSelected());
    editMenu->addSeparator();
    editMenu->addAction(_actions->addFrame());
    editMenu->addAction(_actions->cloneFrame());
    editMenu->addAction(_actions->renameFrame());
    editMenu->addAction(_actions->removeFrame());
    editMenu->addSeparator();
    editMenu->addAction(_actions->addRemoveTileHitbox());
    editMenu->addSeparator();
    editMenu->addAction(_actions->toggleObjSize());
    editMenu->addAction(_actions->flipObjHorizontally());
    editMenu->addAction(_actions->flipObjVertically());
    editMenu->addMenu(_actions->entityHitboxTypeMenu());
    editMenu->addSeparator();
    editMenu->addAction(_actions->addFrameObject());
    editMenu->addAction(_actions->addActionPoint());
    editMenu->addAction(_actions->addEntityHitbox());

    viewMenu->addSeparator();
    _layerSettings->populateMenu(viewMenu);
}

void MainWindow::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->frameMap()->disconnect(this);
    }
    _document = document;

    populateWidgets();

    if (_document) {
        connect(document, &Document::resourceLoaded,
                this, &MainWindow::populateWidgets);
        connect(document->frameMap(), &FrameMap::selectedItemChanged,
                this, &MainWindow::onSelectedFrameChanged);
    }
}

void MainWindow::populateWidgets()
{
    // Widgets cannot handle a null frameSet
    Document* d = _document && _document->frameSet() ? _document : nullptr;

    _frameListModel->setDocument(d);
    _actions->setDocument(d);
    _tilesetPixmaps->setDocument(d);
    _graphicsScene->setDocument(d);
    _animationPreview->setDocument(d);
    _frameSetDock->setDocument(d);
    _frameDock->setDocument(d);
    _animationDock->setDocument(d);
    _palettesDock->setDocument(d);
    _tilesetDock->setDocument(d);

    _tabWidget->setEnabled(d != nullptr);

    onSelectedFrameChanged();
}

void MainWindow::onSelectedFrameChanged()
{
    if (_document && _document->frameMap()->isFrameSelected()) {
        _graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    }
    else {
        _graphicsView->setDragMode(QGraphicsView::NoDrag);
    }
}
