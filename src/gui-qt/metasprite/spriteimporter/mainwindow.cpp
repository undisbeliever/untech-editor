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
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/metasprite/animation/accessors.h"
#include "gui-qt/metasprite/animation/animationdock.h"
#include "gui-qt/metasprite/animation/animationpreview.h"
#include "gui-qt/metasprite/layersettings.h"
#include "models/metasprite/metasprite-error.h"

#include <QPushButton>
#include <QStatusBar>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

MainWindow::MainWindow(ZoomSettingsManager* zoomManager, QWidget* parent)
    : QMainWindow(parent)
    , _document(nullptr)
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

    _graphicsView->addAction(_frameSetDock->addFrameAction());
    for (auto* a : _frameDock->frameContentsContextMenu()->actions()) {
        _graphicsView->addAction(a);
    }
    _frameDock->populateMenu(_graphicsScene->frameContextMenu());

    _graphicsView->setMinimumSize(256, 256);
    _graphicsView->setZoomSettings(zoomManager->get("spriteimporter"));
    _graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);
    _graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    _graphicsView->setScene(_graphicsScene);

    _animationPreview->setZoomSettings(zoomManager->get("metasprite-preview"));
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

    connect(_tabWidget, &QTabWidget::currentChanged,
            this, &MainWindow::currentTabChanged);
}

MainWindow::~MainWindow() = default;

ZoomSettings* MainWindow::zoomSettings() const
{
    if (_tabWidget->currentWidget() == _graphicsView) {
        return _graphicsView->zoomSettings();
    }
    else {
        return _animationPreview->zoomSettings();
    }
}

void MainWindow::populateMenu(QMenu* editMenu, QMenu* viewMenu)
{
    editMenu->addSeparator();
    editMenu->addAction(_frameSetDock->addFrameAction());
    _frameDock->populateMenu(editMenu);

    viewMenu->addSeparator();
    _layerSettings->populateMenu(viewMenu);
}

void MainWindow::setDocument(Document* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->frameList()->disconnect(this);
    }
    _document = document;

    populateWidgets();

    if (document != nullptr) {
        connect(document, &Document::resourceLoaded,
                this, &MainWindow::populateWidgets);

        connect(document->frameList(), &FrameList::selectedIndexChanged,
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

    onSelectedFrameChanged();
}

void MainWindow::onSelectedFrameChanged()
{
    if (_document && _document->frameList()->isSelectedIndexValid()) {
        _graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    }
    else {
        _graphicsView->setDragMode(QGraphicsView::NoDrag);
    }
}

void MainWindow::onErrorDoubleClicked(const UnTech::ErrorListItem& error)
{
    using MetaSpriteError = UnTech::MetaSprite::MetaSpriteError;
    using Type = UnTech::MetaSprite::MsErrorType;

    auto updateSelection = [](auto* accessor, const void* ptr) {
        const auto* list = accessor->list();
        if (list) {
            for (unsigned i = 0; i < list->size(); i++) {
                if (&list->at(i) == ptr) {
                    accessor->setSelectedIndex(i);
                    return;
                }
            }
        }
        accessor->unselectItem();
    };

    if (_document == nullptr) {
        return;
    }

    if (error.specialized == nullptr) {
        _frameSetDock->raise();
    }
    else if (const auto* e = dynamic_cast<const MetaSpriteError*>(error.specialized.get())) {
        switch (e->type()) {
        case Type::FRAME:
        case Type::FRAME_OBJECT:
        case Type::ACTION_POINT:
        case Type::ENTITY_HITBOX:
            updateSelection(_document->frameList(), e->ptr());
            _document->frameList()->setTileHitboxSelected(false);
            _document->frameObjectList()->clearSelection();
            _document->actionPointList()->clearSelection();
            _document->entityHitboxList()->clearSelection();
            showGraphicsTab();
            _frameDock->raise();
            break;

        case Type::ANIMATION:
        case Type::ANIMATION_FRAME:
            updateSelection(_document->animationsList(), e->ptr());
            _animationDock->raise();
            break;
        }

        switch (e->type()) {
        case Type::FRAME:
            break;

        case Type::FRAME_OBJECT:
            _document->frameObjectList()->setSelectedIndexes({ e->id() });
            break;

        case Type::ACTION_POINT:
            _document->actionPointList()->setSelectedIndexes({ e->id() });
            break;

        case Type::ENTITY_HITBOX:
            _document->entityHitboxList()->setSelectedIndexes({ e->id() });
            break;

        case Type::ANIMATION:
            // clear animation frame selection
            _document->animationFramesList()->setSelectedIndex(INT_MAX);
            break;

        case Type::ANIMATION_FRAME:
            _document->animationFramesList()->setSelectedIndex(e->id());
            break;
        }
    }
}

void MainWindow::showGraphicsTab()
{
    _tabWidget->setCurrentWidget(_graphicsView);
}
