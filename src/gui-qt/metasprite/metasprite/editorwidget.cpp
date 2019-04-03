/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "actions.h"
#include "managers.h"
#include "msanimationpreviewitem.h"
#include "msgraphicsscene.h"
#include "palettesdock.h"
#include "resourceitem.h"
#include "tilesetdock.h"
#include "tilesetpixmaps.h"
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
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

EditorWidget::EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent)
    : AbstractEditorWidget(parent)
    , _resourceItem(nullptr)
    , _layerSettings(new LayerSettings(this))
    , _layersButton(new QPushButton(tr("Layers"), this))
    , _tilesetPixmaps(new TilesetPixmaps(this))
    , _frameSetManager(new FrameSetManager(this))
    , _frameManager(new FrameManager(this))
    , _frameObjectManager(new FrameObjectManager(this))
    , _actionPointManager(new ActionPointManager(this))
    , _entityHitboxManager(new EntityHitboxManager(this))
    , _frameListDock(new Accessor::NamedListDock(this))
    , _frameSetDock(createPropertyDockWidget(_frameSetManager, tr("FrameSet"), "MS_FrameSetDock"))
    , _framePropertiesDock(createPropertyDockWidget(_frameManager, tr("Frame"), "MS_FramePropertiesDock"))
    , _frameContentsDock(new Accessor::MultipleSelectionTableDock(
          tr("Frame Contents"),
          { _frameObjectManager, _actionPointManager, _entityHitboxManager },
          { tr("Location"), tr("Parameter"), tr("Tile"), tr("Flip") },
          this))
    , _animationDock(new Animation::AnimationDock(this))
    , _palettesDock(new PalettesDock(this))
    , _tilesetDock(new TilesetDock(_tilesetPixmaps, this))
    , _actions(new Actions(_frameListDock->namedListActions(),
                           _frameContentsDock->viewActions(),
                           _animationDock,
                           this))
    , _tabWidget(new QTabWidget(this))
    , _graphicsView(new ZoomableGraphicsView(this))
    , _graphicsScene(new MsGraphicsScene(_layerSettings, _tilesetPixmaps, this))
    , _animationPreview(new Animation::AnimationPreview(_animationDock, this))
    , _animationPreviewItemFactory(new MsAnimationPreviewItemFactory(_layerSettings, _tilesetPixmaps, this))
{
    QMenu* layerMenu = new QMenu(this);
    _layerSettings->populateMenu(layerMenu);
    _layersButton->setMenu(layerMenu);

    auto* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    this->setLayout(layout);
    layout->addWidget(_tabWidget);

    _actions->populateGraphicsView(_graphicsView);
    _actions->populateGraphicsView(_graphicsScene->contextMenu());
    _actions->populateFrameContentsDockMenu(_frameContentsDock->selectedContextmenu());

    _graphicsView->setMinimumSize(256, 256);
    _graphicsView->setZoomSettings(zoomManager->get("metasprite"));
    _graphicsView->setRubberBandSelectionMode(Qt::ContainsItemShape);
    _graphicsView->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    _graphicsView->setScene(_graphicsScene);

    _animationPreview->setZoomSettings(zoomManager->get("metasprite-preview"));
    _animationPreview->setItemFactory(_animationPreviewItemFactory);

    _tabWidget->setTabPosition(QTabWidget::West);
    _tabWidget->addTab(_graphicsView, tr("Frame"));
    _tabWidget->addTab(_animationPreview, tr("Animation Preview"));

    setResourceItem(nullptr);

    connect(_tabWidget, &QTabWidget::currentChanged,
            this, &EditorWidget::zoomSettingsChanged);

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

    connect(_graphicsScene, &MsGraphicsScene::frameContentSelected,
            _frameContentsDock, &QDockWidget::raise);
}

EditorWidget::~EditorWidget() = default;

QList<QDockWidget*> EditorWidget::createDockWidgets(QMainWindow* mainWindow)
{
    // Ensure docks have a unique name
    _frameListDock->setObjectName("MS_FrameListDock");
    _frameContentsDock->setObjectName("MS_FrameContentsDock");
    _animationDock->setObjectName("MS_AnimationDock");
    _palettesDock->setObjectName("MS_PalettesDock");
    _tilesetDock->setObjectName("MS_TilesetDock");

    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _frameListDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _frameSetDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _framePropertiesDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _frameContentsDock);
    mainWindow->addDockWidget(Qt::RightDockWidgetArea, _animationDock);
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, _palettesDock);
    mainWindow->addDockWidget(Qt::BottomDockWidgetArea, _tilesetDock);

    mainWindow->tabifyDockWidget(_frameSetDock, _framePropertiesDock);
    mainWindow->tabifyDockWidget(_frameSetDock, _frameContentsDock);
    mainWindow->tabifyDockWidget(_frameSetDock, _animationDock);

    mainWindow->resizeDocks({ _frameListDock, _frameSetDock }, { 1, 1000 }, Qt::Vertical);
    mainWindow->resizeDocks({ _palettesDock }, { 1 }, Qt::Vertical);
    mainWindow->resizeDocks({ _palettesDock, _tilesetDock }, { 1, 10000 }, Qt::Horizontal);

    return {
        _tilesetDock,
        _palettesDock,
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

    if (_resourceItem) {
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

    _tilesetPixmaps->setResourceItem(r);
    _graphicsScene->setResourceItem(r);
    _animationPreview->setResourceItem(r);
    _frameListDock->setAccessor(r ? r->frameList() : nullptr);
    _actions->setResourceItem(r);
    _animationDock->setResourceItem(r);
    _palettesDock->setResourceItem(r);
    _tilesetDock->setResourceItem(r);

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
            updateSelection(_resourceItem->frameList(), e->ptr());
            _resourceItem->frameList()->setTileHitboxSelected(false);
            _resourceItem->frameObjectList()->clearSelection();
            _resourceItem->actionPointList()->clearSelection();
            _resourceItem->entityHitboxList()->clearSelection();
            showGraphicsTab();
            break;

        case Type::ANIMATION:
        case Type::ANIMATION_FRAME:
            updateSelection(_resourceItem->animationsList(), e->ptr());
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
            _resourceItem->entityHitboxList()->setSelectedIndexes({ e->id() });
            _frameContentsDock->raise();
            break;

        case Type::ANIMATION:
            // clear animation frame selection
            _resourceItem->animationFramesList()->clearSelection();
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
