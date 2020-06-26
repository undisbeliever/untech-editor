/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "accessors.h"
#include "managers.h"
#include "resourceitem.h"
#include "roomentitiesdock.h"
#include "roomgraphicsscenes.h"
#include "gui-qt/accessor/listaccessortabledock.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/entity/entity-rom-entries/entitieswithiconsmodel.h"
#include "gui-qt/metatiles/mttileset/mttilesetgraphicsscenes.h"
#include "gui-qt/metatiles/mttileset/mttilesetrenderer.h"
#include "gui-qt/metatiles/mttileset/resourcelist.h"
#include "gui-qt/metatiles/style.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/palette/resourcelist.h"
#include "gui-qt/staticresourcelist.h"
#include "models/project/project-data.h"
#include "models/resources/invalid-image-error.h"
#include "models/resources/scenes.h"

#include "gui-qt/rooms/editorwidget.ui.h"

using namespace UnTech::GuiQt::Rooms;

EditorWidget::EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(new Ui::EditorWidget)
    , _dockedMinimapView(new OpenGLZoomableGraphicsView(this))
    , _dockedTilesetView(new OpenGLZoomableGraphicsView(this))
    , _dockedScratchpadView(new OpenGLZoomableGraphicsView(this))
    , _style(new MetaTiles::Style(this))
    , _renderer(new MetaTiles::MtTileset::MtTilesetRenderer(this))
    , _editableRoomScene(new EditableRoomGraphicsScene(_style, _renderer, this))
    , _minimapRoomScene(new RoomGraphicsScene(_style, _renderer, this))
    , _tilesetScene(new MetaTiles::MtTileset::MtTilesetGraphicsScene(_style, _renderer, this))
    , _scratchpadScene(new MetaTiles::MtTileset::MtScratchpadGraphicsScene(_style, _renderer, this))
    , _propertyManager(new RoomPropertyManager(this))
    , _roomEntranceManager(new RoomEntranceManager(this))
    , _entitiesWithIconsModel(new Entity::EntityRomEntries::EntitiesWithIconsModel(this))
    , _entitiesListView(new QListView(this))
    , _propertyDock(createPropertyDockWidget(_propertyManager, tr("Properties"), QStringLiteral("Properties")))
    , _minimapDock(createDockWidget(_dockedMinimapView, tr("Minimap"), QStringLiteral("Minimap")))
    , _tilesetDock(createDockWidget(_dockedTilesetView, tr("MetaTiles"), QStringLiteral("MetaTiles")))
    , _scratchpadDock(createDockWidget(_dockedScratchpadView, tr("Scratchpad"), QStringLiteral("Scratchpad")))
    , _entitiesListDock(createDockWidget(_entitiesListView, tr("Entities"), QStringLiteral("EntitiesList")))
    , _roomEntitiesDock(new RoomEntitiesDock(this))
    , _roomEntrancesDock(new Accessor::ListAccessorTableDock(tr("Room Entrances"), _roomEntranceManager, this))
    , _resourceItem(nullptr)
{
    using MtTilesetRenderer = UnTech::GuiQt::MetaTiles::MtTileset::MtTilesetRenderer;

    _ui->setupUi(this);

    _editableRoomScene->populateActions(_ui->editorToolBar);

    _renderer->setPlayButton(_ui->playButton);
    _renderer->setRegionCombo(_ui->region);

    _editableRoomScene->addGridSelectionSource(_minimapRoomScene);
    _editableRoomScene->addGridSelectionSource(_scratchpadScene);
    _editableRoomScene->addGridSelectionSource(_tilesetScene);

    _ui->graphicsView->setZoomSettings(zoomManager->get("room"));
    _ui->graphicsView->setScene(_editableRoomScene);

    _dockedMinimapView->setScene(_minimapRoomScene);
    _dockedTilesetView->setScene(_tilesetScene);
    _dockedScratchpadView->setScene(_scratchpadScene);

    // Fit entire tileset width in view
    _dockedTilesetView->setMinimumWidth(16 * 16 + 8);

    _entitiesListView->setModel(_entitiesWithIconsModel);
    _entitiesListView->setViewMode(QListView::ListMode);
    _entitiesListView->setTextElideMode(Qt::ElideRight);
    _entitiesListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _entitiesListView->setDragEnabled(true);

    // Simplifies MainWindow state
    _roomEntitiesDock->setObjectName(QStringLiteral("RoomEntities"));
    _roomEntrancesDock->setObjectName(QStringLiteral("RoomEntrances"));

    this->addDockWidget(Qt::RightDockWidgetArea, _propertyDock);
    this->addDockWidget(Qt::RightDockWidgetArea, _tilesetDock);
    this->addDockWidget(Qt::RightDockWidgetArea, _minimapDock);
    this->addDockWidget(Qt::RightDockWidgetArea, _scratchpadDock);
    this->addDockWidget(Qt::RightDockWidgetArea, _entitiesListDock);
    this->addDockWidget(Qt::RightDockWidgetArea, _roomEntitiesDock);
    this->addDockWidget(Qt::RightDockWidgetArea, _roomEntrancesDock);

    this->tabifyDockWidget(_minimapDock, _scratchpadDock);
    this->tabifyDockWidget(_roomEntitiesDock, _roomEntrancesDock);
    _scratchpadDock->raise();
    _roomEntitiesDock->raise();

    this->resizeDocks({ _propertyDock, _tilesetDock, _scratchpadDock, _entitiesListDock, _roomEntitiesDock }, { 100, 200, 200, 100, 100 }, Qt::Vertical);

    setEnabled(false);

    connect(_ui->resetAnimationButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::resetAnimations);
    connect(_ui->nextTilesetFrameButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::pauseAndAdvanceTilesetFrame);
    connect(_ui->nextPaletteFrameButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::pauseAndAdvancePaletteFrame);
}

EditorWidget::~EditorWidget() = default;

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("Room");
}

void EditorWidget::populateMenu(QMenu* editMenu, QMenu* viewMenu)
{
    editMenu->addSeparator();
    _editableRoomScene->populateActions(editMenu);

    viewMenu->addSeparator();
    _style->populateActions(viewMenu);
}

UnTech::GuiQt::ZoomSettings* EditorWidget::zoomSettings() const
{
    return _ui->graphicsView->zoomSettings();
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    if (_resourceItem == item) {
        return item != nullptr;
    }

    if (_resourceItem) {
        _resourceItem->disconnect(this);
    }
    _resourceItem = item;

    _propertyManager->setResourceItem(item);
    _roomEntranceManager->setResourceItem(item);
    _editableRoomScene->setResourceItem(item);
    _minimapRoomScene->setResourceItem(item);
    _roomEntitiesDock->setResourceItem(item);

    auto* entities = _resourceItem ? _resourceItem->project()->staticResources()->entities() : nullptr;
    _entitiesWithIconsModel->setResourceItem(entities);

    updateTilesetAndPalette();

    if (_resourceItem) {
        connect(_resourceItem, &ResourceItem::sceneChanged,
                this, &EditorWidget::updateTilesetAndPalette);
    }

    setEnabled(item != nullptr);

    return item != nullptr;
}

void EditorWidget::onErrorDoubleClicked(const UnTech::ErrorListItem& error)
{
    if (_resourceItem == nullptr) {
        return;
    }

    _resourceItem->clearSelection();

    if (error.specialized == nullptr) {
        _propertyDock->raise();
    }
    else if (const auto* e = dynamic_cast<const ListItemError*>(error.specialized.get())) {
        const bool isRoomEntrance = _resourceItem->roomEntrances()->setSelected_Ptr(e->ptr());
        const bool isEntityGroup = _resourceItem->entityGroups()->setSelected_Ptr(e->ptr());
        const bool isEntityEntry = _resourceItem->entityEntries()->setSelected_Ptr(e->ptr());

        if (isRoomEntrance) {
            _roomEntrancesDock->raise();
        }
        else if (isEntityGroup || isEntityEntry) {
            _roomEntitiesDock->raise();
            _roomEntitiesDock->raise();
        }
    }
}

void EditorWidget::updateTilesetAndPalette()
{
    MetaTiles::MtTileset::ResourceItem* tileset = nullptr;
    Resources::Palette::ResourceItem* palette = nullptr;

    if (_resourceItem) {
        if (auto ri = _resourceItem->roomInput()) {
            const auto* project = _resourceItem->project();
            const auto& projectData = project->projectData();

            if (auto compiledScenes = projectData.scenes()) {
                if (auto scene = compiledScenes->findScene(ri->scene)) {
                    if (scene->palette) {
                        palette = qobject_cast<Resources::Palette::ResourceItem*>(project->palettes()->items().value(*scene->palette));
                    }
                    if (scene->mtTileset) {
                        tileset = qobject_cast<MetaTiles::MtTileset::ResourceItem*>(project->mtTilesets()->items().value(*scene->mtTileset));
                    }
                }
            }
        }
    }

    _renderer->setTilesetItem(tileset);
    _renderer->setPaletteItem(palette);
}
