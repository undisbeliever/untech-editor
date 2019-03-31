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
    , _document(nullptr)
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
    auto* document = qobject_cast<Document*>(item);

    if (_document) {
        _document->disconnect(this);
        _document->frameList()->disconnect(this);
    }
    _document = document;

    populateWidgets();

    if (document != nullptr) {
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

    _graphicsScene->setDocument(d);
    _animationPreview->setDocument(d);
    _frameListDock->setAccessor(d ? d->frameList() : nullptr);
    _animationDock->setDocument(d);

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

    auto updateSelection = [](auto* accessor, const MetaSpriteError& e) {
        // Include matching name in SpriteImporter GUI
        // as the error could be about the transformed msFrameSet
        // and `e.ptr()` would no-longer match.

        unsigned nameMatchCount = 0;
        size_t nameMatchIndex = INT_MAX;

        const auto* list = accessor->list();
        if (list) {
            for (unsigned i = 0; i < list->size(); i++) {
                if (&list->at(i) == e.ptr()) {
                    accessor->setSelectedIndex(i);
                    return;
                }
                if (list->at(i).name == e.name()) {
                    nameMatchIndex = i;
                    nameMatchCount++;
                }
            }
        }

        if (nameMatchCount != 1) {
            nameMatchIndex = INT_MAX;
        }
        accessor->setSelectedIndex(nameMatchIndex);
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
            updateSelection(_document->frameList(), *e);
            _document->frameList()->setTileHitboxSelected(false);
            _document->frameObjectList()->clearSelection();
            _document->actionPointList()->clearSelection();
            _document->entityHitboxList()->clearSelection();
            showGraphicsTab();
            break;

        case Type::ANIMATION:
        case Type::ANIMATION_FRAME:
            updateSelection(_document->animationsList(), *e);
            _animationDock->raise();
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
            _document->entityHitboxList()->clearSelection();
            _frameContentsDock->raise();
            break;

        case Type::ANIMATION:
            // clear animation frame selection
            _document->animationFramesList()->setSelectedIndexes({ INT_MAX });
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
