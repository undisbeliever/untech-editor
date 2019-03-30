/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "actions.h"
#include "document.h"
#include "managers.h"
#include "msanimationpreviewitem.h"
#include "msgraphicsscene.h"
#include "palettesdock.h"
#include "tilesetdock.h"
#include "tilesetpixmaps.h"
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
    , _document(nullptr)
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
    auto* document = qobject_cast<Document*>(item);

    if (_document) {
        _document->disconnect(this);
        _document->frameList()->disconnect(this);
    }
    _document = document;

    populateWidgets();

    if (_document) {
        connect(document, &Document::resourceLoaded,
                this, &EditorWidget::populateWidgets);
        connect(document->frameList(), &FrameList::selectedIndexChanged,
                this, &EditorWidget::onSelectedFrameChanged);
    }

    return document != nullptr;
}

void EditorWidget::populateWidgets()
{
    // Widgets cannot handle a null frameSet
    Document* d = _document && _document->frameSet() ? _document : nullptr;

    _frameSetManager->setDocument(d);
    _frameManager->setDocument(d);
    _frameObjectManager->setDocument(d);
    _actionPointManager->setDocument(d);
    _entityHitboxManager->setDocument(d);
    _actions->setDocument(d);

    _tilesetPixmaps->setDocument(d);
    _graphicsScene->setDocument(d);
    _animationPreview->setDocument(d);
    _frameListDock->setAccessor(d ? d->frameList() : nullptr);
    _actions->setDocument(d);
    _animationDock->setDocument(d);
    _palettesDock->setDocument(d);
    _tilesetDock->setDocument(d);

    _tabWidget->setEnabled(d != nullptr);

    onSelectedFrameChanged();
}

void EditorWidget::onSelectedFrameChanged()
{
    if (_document && _document->frameList()->isSelectedIndexValid()) {
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
            break;

        case Type::ANIMATION:
        case Type::ANIMATION_FRAME:
            updateSelection(_document->animationsList(), e->ptr());
            break;
        }

        switch (e->type()) {
        case Type::FRAME:
            _framePropertiesDock->raise();
            break;

        case Type::FRAME_OBJECT:
            _document->frameObjectList()->setSelectedIndexes({ e->id() });
            _frameContentsDock->raise();
            break;

        case Type::ACTION_POINT:
            _document->actionPointList()->setSelectedIndexes({ e->id() });
            _frameContentsDock->raise();
            break;

        case Type::ENTITY_HITBOX:
            _document->entityHitboxList()->setSelectedIndexes({ e->id() });
            _frameContentsDock->raise();
            break;

        case Type::ANIMATION:
            // clear animation frame selection
            _document->animationFramesList()->clearSelection();
            _animationDock->raise();
            break;

        case Type::ANIMATION_FRAME:
            _document->animationFramesList()->setSelectedIndexes({ e->id() });
            _animationDock->raise();
            break;
        }
    }
}

void EditorWidget::showGraphicsTab()
{
    _tabWidget->setCurrentWidget(_graphicsView);
}
