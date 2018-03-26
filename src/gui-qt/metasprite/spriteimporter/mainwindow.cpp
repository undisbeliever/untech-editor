/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "actions.h"
#include "document.h"
#include "framedock.h"
#include "framesetdock.h"
#include "sianimationpreviewitem.h"
#include "sigraphicsscene.h"
#include "gui-qt/common/graphics/zoomablegraphicsview.h"
#include "gui-qt/common/graphics/zoomsettings.h"
#include "gui-qt/metasprite/animation/animationdock.h"
#include "gui-qt/metasprite/animation/animationpreview.h"
#include "gui-qt/metasprite/layersettings.h"

#include <QPushButton>
#include <QStatusBar>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

MainWindow::MainWindow(ZoomSettings* zoomSettings, QWidget* parent)
    : QMainWindow(parent)
    , _document(nullptr)
    , _actions(new Actions(this))
    , _layerSettings(new LayerSettings(this))
    , _imageFileWatcher()
    , _animationPreviewItemFactory(
          new SiAnimationPreviewItemFactory(_layerSettings, this))
{
    // Have the left and right docks take up the whole height of the documentWindow
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    _tabWidget = new QTabWidget(this);
    setCentralWidget(_tabWidget);
    _tabWidget->setTabPosition(QTabWidget::West);

    _graphicsView = new ZoomableGraphicsView(this);
    _graphicsView->setMinimumSize(256, 256);
    _graphicsView->setZoomSettings(zoomSettings);
    _graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);
    _graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);

    _graphicsScene = new SiGraphicsScene(_actions, _layerSettings, this);
    _graphicsView->setScene(_graphicsScene);
    _tabWidget->addTab(_graphicsView, tr("Frame"));

    _animationPreview = new Animation::AnimationPreview(this);
    _animationPreview->setZoomSettings(zoomSettings);
    _animationPreview->setItemFactory(_animationPreviewItemFactory);
    _tabWidget->addTab(_animationPreview, tr("Animation Preview"));

    _frameSetDock = new FrameSetDock(_actions, this);
    addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);

    _frameDock = new FrameDock(_actions, this);
    addDockWidget(Qt::RightDockWidgetArea, _frameDock);

    _animationDock = new Animation::AnimationDock(this);
    addDockWidget(Qt::RightDockWidgetArea, _animationDock);

    tabifyDockWidget(_frameSetDock, _frameDock);
    tabifyDockWidget(_frameSetDock, _animationDock);

    setDocument(nullptr);

    _frameSetDock->raise();

    connect(&_imageFileWatcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::onImageFileChanged);
}

MainWindow::~MainWindow() = default;

void MainWindow::setupMenubar(QMenu* editMenu, QMenu* viewMenu)
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
    editMenu->addMenu(_actions->entityHitboxTypeMenu());
    editMenu->addSeparator();
    editMenu->addAction(_actions->addFrameObject());
    editMenu->addAction(_actions->addActionPoint());
    editMenu->addAction(_actions->addEntityHitbox());

    viewMenu->addSeparator();
    _layerSettings->populateMenu(viewMenu);
}

void MainWindow::setupStatusbar(QStatusBar* statusBar)
{
    QPushButton* layerButton = new QPushButton(tr("Layers"), this);
    QMenu* layerMenu = new QMenu(this);
    _layerSettings->populateMenu(layerMenu);
    layerButton->setMenu(layerMenu);
    statusBar->addPermanentWidget(layerButton);
}

void MainWindow::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    _actions->setDocument(document);
    _graphicsScene->setDocument(document);
    _animationPreview->setDocument(document);
    _frameSetDock->setDocument(document);
    _frameDock->setDocument(document);
    _animationDock->setDocument(document);

    _tabWidget->setEnabled(document != nullptr);

    onFrameSetImageFilenameChanged();

    if (document != nullptr) {
        connect(document, &Document::frameSetImageFilenameChanged,
                this, &MainWindow::onFrameSetImageFilenameChanged);
        connect(document->selection(), &Selection::selectedFrameChanged,
                this, &MainWindow::onSelectedFrameChanged);
    }

    onSelectedFrameChanged();
}

void MainWindow::onSelectedFrameChanged()
{
    if (_document && _document->selection()->hasSelectedFrame()) {
        _graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    }
    else {
        _graphicsView->setDragMode(QGraphicsView::NoDrag);
    }
}

void MainWindow::onFrameSetImageFilenameChanged()
{
    auto removePaths = [this](const auto& list) {
        if (!list.isEmpty()) {
            _imageFileWatcher.removePaths(list);
        }
    };
    removePaths(_imageFileWatcher.files());
    removePaths(_imageFileWatcher.directories());

    if (_document) {
        QString fn = QString::fromStdString(_document->frameSet()->imageFilename);
        if (!fn.isEmpty()) {
            _imageFileWatcher.addPath(fn);
        }
    }
}

void MainWindow::onImageFileChanged()
{
    Q_ASSERT(_document != nullptr);

    _document->frameSet()->reloadImage();

    emit _document->frameSetImageChanged();
}
