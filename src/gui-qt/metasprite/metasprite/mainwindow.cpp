/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "actions.h"
#include "document.h"
#include "framedock.h"
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

MainWindow::MainWindow(QWidget* parent)
    : AbstractMainWindow(parent)
    , _actions(new Actions(this))
    , _zoomSettings(new ZoomSettings(6.0, ZoomSettings::NTSC, this))
    , _layerSettings(new LayerSettings(this))
    , _tilesetPixmaps(new TilesetPixmaps(this))
    , _animationPreviewItemFactory(
          new MsAnimationPreviewItemFactory(_layerSettings, _tilesetPixmaps, this))
{
    _tabWidget = new QTabWidget(this);
    setCentralWidget(_tabWidget);
    _tabWidget->setTabPosition(QTabWidget::West);

    _graphicsView = new ZoomableGraphicsView(this);
    _graphicsView->setMinimumSize(256, 256);
    _graphicsView->setZoomSettings(_zoomSettings);
    _graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);
    _graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);

    _graphicsScene = new MsGraphicsScene(_actions, _layerSettings,
                                         _tilesetPixmaps, this);
    _graphicsView->setScene(_graphicsScene);
    _tabWidget->addTab(_graphicsView, tr("Frame"));

    _animationPreview = new Animation::AnimationPreview(this);
    _animationPreview->setZoomSettings(_zoomSettings);
    _animationPreview->setItemFactory(_animationPreviewItemFactory);
    _tabWidget->addTab(_animationPreview, tr("Animation Preview"));

    _frameSetDock = new FrameSetDock(_actions, this);
    addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);

    _frameDock = new FrameDock(_actions, this);
    addDockWidget(Qt::RightDockWidgetArea, _frameDock);

    _animationDock = new Animation::AnimationDock(this);
    addDockWidget(Qt::RightDockWidgetArea, _animationDock);

    _palettesDock = new PalettesDock(_actions, this);
    addDockWidget(Qt::BottomDockWidgetArea, _palettesDock);

    _tilesetDock = new TilesetDock(_tilesetPixmaps, this);
    addDockWidget(Qt::BottomDockWidgetArea, _tilesetDock);

    tabifyDockWidget(_frameSetDock, _frameDock);
    tabifyDockWidget(_frameSetDock, _animationDock);

    resizeDocks({ _palettesDock }, { 1 }, Qt::Vertical);

    documentChangedEvent(nullptr, nullptr);

    setupMenubar();
    setupStatusbar();
    readSettings();

    _frameSetDock->raise();
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenubar()
{
    _editMenu->addSeparator();
    _editMenu->addAction(_actions->raiseSelected());
    _editMenu->addAction(_actions->lowerSelected());
    _editMenu->addAction(_actions->cloneSelected());
    _editMenu->addAction(_actions->removeSelected());
    _editMenu->addSeparator();
    _editMenu->addAction(_actions->addFrame());
    _editMenu->addAction(_actions->cloneFrame());
    _editMenu->addAction(_actions->renameFrame());
    _editMenu->addAction(_actions->removeFrame());
    _editMenu->addSeparator();
    _editMenu->addAction(_actions->addRemoveTileHitbox());
    _editMenu->addSeparator();
    _editMenu->addAction(_actions->toggleObjSize());
    _editMenu->addAction(_actions->flipObjHorizontally());
    _editMenu->addAction(_actions->flipObjVertically());
    _editMenu->addMenu(_actions->entityHitboxTypeMenu());
    _editMenu->addSeparator();
    _editMenu->addAction(_actions->addFrameObject());
    _editMenu->addAction(_actions->addActionPoint());
    _editMenu->addAction(_actions->addEntityHitbox());

    _zoomSettings->populateMenu(_viewMenu);
    _viewMenu->addSeparator();
    _layerSettings->populateMenu(_viewMenu);
}

void MainWindow::setupStatusbar()
{
    QPushButton* layerButton = new QPushButton(tr("Layers"), this);
    QMenu* layerMenu = new QMenu(this);
    _layerSettings->populateMenu(layerMenu);
    layerButton->setMenu(layerMenu);
    statusBar()->addPermanentWidget(layerButton);

    _aspectRatioComboBox = new QComboBox(this);
    _zoomSettings->setAspectRatioComboBox(_aspectRatioComboBox);
    statusBar()->addPermanentWidget(_aspectRatioComboBox);

    _zoomComboBox = new QComboBox(this);
    _zoomSettings->setZoomComboBox(_zoomComboBox);
    statusBar()->addPermanentWidget(_zoomComboBox);
}

std::unique_ptr<AbstractDocument> MainWindow::createDocumentInstance()
{
    return std::make_unique<Document>();
}

void MainWindow::documentChangedEvent(AbstractDocument* abstractDocument,
                                      AbstractDocument* abstractOldDocument)
{
    Document* document = qobject_cast<Document*>(abstractDocument);
    Document* oldDocument = qobject_cast<Document*>(abstractOldDocument);

    if (oldDocument) {
        oldDocument->selection()->disconnect(this);
    }

    _actions->setDocument(document);
    _tilesetPixmaps->setDocument(document);
    _graphicsScene->setDocument(document);
    _animationPreview->setDocument(document);
    _frameSetDock->setDocument(document);
    _frameDock->setDocument(document);
    _animationDock->setDocument(document);
    _palettesDock->setDocument(document);
    _tilesetDock->setDocument(document);

    _tabWidget->setEnabled(document != nullptr);

    if (document != nullptr) {
        connect(document->selection(), &Selection::selectedFrameChanged,
                this, &MainWindow::onSelectedFrameChanged);
    }

    onSelectedFrameChanged();
}

void MainWindow::onSelectedFrameChanged()
{
    Document* document = qobject_cast<Document*>(this->document());

    if (document && document->selection()->hasSelectedFrame()) {
        _graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    }
    else {
        _graphicsView->setDragMode(QGraphicsView::NoDrag);
    }
}
