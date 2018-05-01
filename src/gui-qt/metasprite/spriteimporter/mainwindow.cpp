/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mainwindow.h"
#include "accessors.h"
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
    , _imageFileWatcher()
    , _layerSettings(new LayerSettings(this))
    , _layersButton(new QPushButton(tr("Layers"), this))
    , _frameSetDock(new FrameSetDock(this))
    , _frameDock(new FrameDock(_frameSetDock->frameListModel(), this))
    , _animationDock(new Animation::AnimationDock(this))
    , _tabWidget(new QTabWidget(this))
    , _graphicsView(new ZoomableGraphicsView(this))
    , _graphicsScene(new SiGraphicsScene(_layerSettings, this))
    , _animationPreview(new Animation::AnimationPreview(_animationDock, this))
    , _animationPreviewItemFactory(new SiAnimationPreviewItemFactory(_layerSettings, this))
{
    // Have the left and right docks take up the whole height of the documentWindow
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
    setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    QMenu* layerMenu = new QMenu(this);
    _layerSettings->populateMenu(layerMenu);
    _layersButton->setMenu(layerMenu);

    _frameDock->populateMenu(_graphicsScene->frameContextMenu());

    _graphicsView->setMinimumSize(256, 256);
    _graphicsView->setZoomSettings(zoomSettings);
    _graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);
    _graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    _graphicsView->setScene(_graphicsScene);

    _animationPreview->setZoomSettings(zoomSettings);
    _animationPreview->setItemFactory(_animationPreviewItemFactory);

    _tabWidget->setTabPosition(QTabWidget::West);
    _tabWidget->addTab(_graphicsView, tr("Frame"));
    _tabWidget->addTab(_animationPreview, tr("Animation Preview"));
    setCentralWidget(_tabWidget);

    addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);
    addDockWidget(Qt::RightDockWidgetArea, _frameDock);
    addDockWidget(Qt::RightDockWidgetArea, _animationDock);

    tabifyDockWidget(_frameSetDock, _frameDock);
    tabifyDockWidget(_frameSetDock, _animationDock);

    setDocument(nullptr);

    _frameSetDock->raise();

    connect(&_imageFileWatcher, &QFileSystemWatcher::fileChanged,
            this, &MainWindow::onImageFileChanged);
}

MainWindow::~MainWindow() = default;

void MainWindow::populateMenu(QMenu* editMenu, QMenu* viewMenu)
{
    editMenu->addSeparator();
    _frameSetDock->populateMenu(editMenu);
    editMenu->addSeparator();
    _frameDock->populateMenu(editMenu);
    editMenu->addSeparator();

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

    if (document != nullptr) {
        connect(document, &Document::resourceLoaded,
                this, &MainWindow::populateWidgets);
        connect(document, &Document::frameSetImageFilenameChanged,
                this, &MainWindow::onFrameSetImageFilenameChanged);
        connect(document->frameMap(), &FrameMap::selectedItemChanged,
                this, &MainWindow::onSelectedFrameChanged);
    }
}

void MainWindow::populateWidgets()
{
    // Widgets cannot handle a null frameSet
    Document* d = _document && _document->frameSet() ? _document : nullptr;

    _graphicsScene->setDocument(d);
    _animationPreview->setDocument(d);
    _frameSetDock->setDocument(d);
    _frameDock->setDocument(d);
    _animationDock->setDocument(d);

    _tabWidget->setEnabled(d != nullptr);

    onFrameSetImageFilenameChanged();
    onSelectedFrameChanged();
}

void MainWindow::onSelectedFrameChanged()
{
    if (_document && _document->frameMap()->selectedFrame() != nullptr) {
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

    if (_document && _document->frameSet()) {
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
