/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "actions.h"
#include "managers.h"
#include "resourceitem.h"
#include "sianimationpreviewitem.h"
#include "sigraphicsscene.h"
#include "gui-qt/accessor/listactions.h"
#include "gui-qt/accessor/multipleselectiontabledock.h"
#include "gui-qt/accessor/namedlistdock.h"
#include "gui-qt/common/graphics/zoomablegraphicsview.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/metasprite/animation/accessors.h"
#include "gui-qt/metasprite/animation/animationdock.h"
#include "gui-qt/metasprite/animation/animationpreview.h"
#include "gui-qt/metasprite/layersettings.h"
#include "models/metasprite/metasprite-error.h"

#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

EditorWidget::EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent)
    : AbstractEditorWidget(parent)
    , _resourceItem(nullptr)
    , _layerSettings(new LayerSettings(this))
    , _layersButton(new QPushButton(tr("Layers"), this))
    , _frameSetManager(new FrameSetManager(this))
    , _frameManager(new FrameManager(this))
    , _frameObjectManager(new FrameObjectManager(this))
    , _actionPointManager(new ActionPointManager(this))
    , _entityHitboxManager(new EntityHitboxManager(this))
    , _frameListDock(new Accessor::NamedListDock(this))
    , _frameSetDock(createPropertyDockWidget(_frameSetManager, tr("FrameSet"), "SI_FrameSetDock"))
    , _framePropertiesDock(createPropertyDockWidget(_frameManager, tr("Frame"), "SI_FramePropertiesDock"))
    , _frameContentsDock(new Accessor::MultipleSelectionTableDock(
          tr("Frame Contents"),
          { _frameObjectManager, _actionPointManager, _entityHitboxManager },
          { tr("Location"), tr("Parameter") },
          this))
    , _animationDock(new Animation::AnimationDock(this))
    , _actions(new Actions(_frameListDock->namedListActions(),
                           _frameContentsDock->viewActions(),
                           _animationDock,
                           this))
    , _tabWidget(new QTabWidget(this))
    , _graphicsView(new ZoomableGraphicsView(this))
    , _graphicsScene(new SiGraphicsScene(_layerSettings, this))
    , _animationPreview(new Animation::AnimationPreview(_animationDock, this))
    , _animationPreviewItemFactory(new SiAnimationPreviewItemFactory(_layerSettings, this))
{
    QMenu* layerMenu = new QMenu(this);
    _layerSettings->populateMenu(layerMenu);
    _layersButton->setMenu(layerMenu);

    auto* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    this->setLayout(layout);
    layout->addWidget(_tabWidget);

    _tabWidget->setTabPosition(QTabWidget::West);

    auto* frameListActions = _frameListDock->namedListActions();
    frameListActions->add->setShortcut(Qt::CTRL + Qt::Key_N);

    _actions->populateGraphicsView(_graphicsView);
    _actions->populateGraphicsView(_graphicsScene->frameContextMenu());
    _actions->populateFrameContentsDockMenu(_frameContentsDock->selectedContextmenu());

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

    setResourceItem(nullptr);

    _frameSetDock->raise();

    connect(_tabWidget, &QTabWidget::currentChanged,
            this, &EditorWidget::currentTabChanged);

    // Raise appropriate dock when adding/cloning a new item
    connect(_actions->frameListActions->add, &QAction::triggered,
            _framePropertiesDock, &QDockWidget::raise);
    connect(_actions->frameListActions->clone, &QAction::triggered,
            _framePropertiesDock, &QDockWidget::raise);
    for (QAction* a : _actions->frameContentsActions->addActions()) {
        connect(a, &QAction::triggered,
                _frameContentsDock, &QDockWidget::raise);
    }
    connect(_actions->frameContentsActions->clone, &QAction::triggered,
            _frameContentsDock, &QDockWidget::raise);
    connect(_actions->animationListActions->add, &QAction::triggered,
            _animationDock, &QDockWidget::raise);
    connect(_actions->animationListActions->clone, &QAction::triggered,
            _animationDock, &QDockWidget::raise);
    connect(_actions->animationFrameActions->add, &QAction::triggered,
            _animationDock, &QDockWidget::raise);
    connect(_actions->animationFrameActions->clone, &QAction::triggered,
            _animationDock, &QDockWidget::raise);

    connect(_graphicsScene, &SiGraphicsScene::frameContentSelected,
            _frameContentsDock, &QDockWidget::raise);
}

EditorWidget::~EditorWidget() = default;

QList<QDockWidget*> EditorWidget::createDockWidgets(QMainWindow* mainWindow)
{
    _frameListDock->setObjectName("SI_FrameListDock");
    _frameContentsDock->setObjectName("SI_FrameContentsDock");
    _animationDock->setObjectName("SI_AnimationDock");

    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _frameListDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _framePropertiesDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _frameContentsDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _animationDock);

    mainWindow->tabifyDockWidget(_frameSetDock, _framePropertiesDock);
    mainWindow->tabifyDockWidget(_frameSetDock, _frameContentsDock);
    mainWindow->tabifyDockWidget(_frameSetDock, _animationDock);

    mainWindow->resizeDocks({ _frameListDock, _frameSetDock }, { 1, 1000 }, Qt::Vertical);

    return {
        _animationDock,
        _frameContentsDock,
        _framePropertiesDock,
        _frameSetDock,
        _frameListDock,
    };
}

QPushButton* EditorWidget::statusBarWidget() const
{
    return _layersButton;
}

ZoomSettings* EditorWidget::zoomSettings() const
{
    if (_tabWidget->currentWidget() == _graphicsView) {
        return _graphicsView->zoomSettings();
    }
    else {
        return _animationPreview->zoomSettings();
    }
}

void EditorWidget::populateMenu(QMenu* editMenu, QMenu* viewMenu)
{
    _actions->populateEditMenu(editMenu);

    viewMenu->addSeparator();
    _layerSettings->populateMenu(viewMenu);
}

bool EditorWidget::setResourceItem(AbstractResourceItem* item)
{
    auto* resourceItem = qobject_cast<ResourceItem*>(item);

    if (_resourceItem) {
        _resourceItem->disconnect(this);
        _resourceItem->frameList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    populateWidgets();

    if (resourceItem != nullptr) {
        connect(resourceItem, &ResourceItem::resourceLoaded,
                this, &EditorWidget::populateWidgets);

        connect(resourceItem->frameList(), &FrameList::selectedIndexChanged,
                this, &EditorWidget::onSelectedFrameChanged);
    }

    return resourceItem != nullptr;
}

void EditorWidget::populateWidgets()
{
    // Widgets cannot handle a null frameSet
    ResourceItem* r = _resourceItem && _resourceItem->frameSet() ? _resourceItem : nullptr;

    _frameSetManager->setResourceItem(r);
    _frameManager->setResourceItem(r);
    _frameObjectManager->setResourceItem(r);
    _actionPointManager->setResourceItem(r);
    _entityHitboxManager->setResourceItem(r);
    _actions->setResourceItem(r);

    _graphicsScene->setResourceItem(r);
    _animationPreview->setResourceItem(r);
    _frameListDock->setAccessor(r ? r->frameList() : nullptr);
    _animationDock->setResourceItem(r);

    _tabWidget->setEnabled(r != nullptr);

    onSelectedFrameChanged();
}

void EditorWidget::onSelectedFrameChanged()
{
    if (_resourceItem && _resourceItem->frameList()->isSelectedIndexValid()) {
        _graphicsView->setDragMode(QGraphicsView::RubberBandDrag);
    }
    else {
        _graphicsView->setDragMode(QGraphicsView::NoDrag);
    }

    _frameContentsDock->expandAll();
}

void EditorWidget::onErrorDoubleClicked(const UnTech::ErrorListItem& error)
{
    using MetaSpriteError = UnTech::MetaSprite::MetaSpriteError;
    using Type = UnTech::MetaSprite::MsErrorType;

    auto updateSelection = [](auto* accessor, const MetaSpriteError& e) {
        // Include matching name in SpriteImporter GUI
        // as the error could be about the transformed msFrameSet
        // and `e.ptr()` would no-longer match.

        bool foundPtr = accessor->setSelected_Ptr(e.ptr());
        if (not foundPtr) {
            accessor->setSelected_Name(e.name());
        }
    };

    if (_resourceItem == nullptr) {
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
            updateSelection(_resourceItem->frameList(), *e);
            _resourceItem->frameList()->setTileHitboxSelected(false);
            _resourceItem->frameObjectList()->clearSelection();
            _resourceItem->actionPointList()->clearSelection();
            _resourceItem->entityHitboxList()->clearSelection();
            showGraphicsTab();
            break;

        case Type::ANIMATION:
        case Type::ANIMATION_FRAME:
            updateSelection(_resourceItem->animationsList(), *e);
            _animationDock->raise();
            break;
        }

        switch (e->type()) {
        case Type::FRAME:
            _framePropertiesDock->raise();
            break;

        case Type::FRAME_OBJECT:
            _resourceItem->frameObjectList()->setSelectedIndexes({ e->id() });
            _frameContentsDock->raise();
            break;

        case Type::ACTION_POINT:
            _resourceItem->actionPointList()->setSelectedIndexes({ e->id() });
            _frameContentsDock->raise();
            break;

        case Type::ENTITY_HITBOX:
            _resourceItem->entityHitboxList()->clearSelection();
            _frameContentsDock->raise();
            break;

        case Type::ANIMATION:
            // clear animation frame selection
            _resourceItem->animationFramesList()->setSelectedIndexes({ INT_MAX });
            _animationDock->raise();
            break;

        case Type::ANIMATION_FRAME:
            _resourceItem->animationFramesList()->setSelectedIndexes({ e->id() });
            _animationDock->raise();
            break;
        }
    }
}

void EditorWidget::showGraphicsTab()
{
    _tabWidget->setCurrentWidget(_graphicsView);
}
