/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "managers.h"
#include "resourceitem.h"
#include "roomentitiesdock.h"
#include "roomgraphicsscenes.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/common/properties/propertylistview.h"
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

#include <QListView>

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
    , _roomEntitiesDock(new RoomEntitiesDock(this))
    , _propertyManager(new RoomPropertyManager(this))
    , _entitiesWithIconsModel(new Entity::EntityRomEntries::EntitiesWithIconsModel(this))
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

    auto* propertyDock = createPropertyDockWidget(_propertyManager, tr("Properties"), QStringLiteral("Properties"));
    auto* minimapDock = createDockWidget(_dockedMinimapView, tr("Minimap"), QStringLiteral("Minimap"));
    auto* tilesetDock = createDockWidget(_dockedTilesetView, tr("MetaTiles"), QStringLiteral("MetaTiles"));
    auto* scratchpadDock = createDockWidget(_dockedScratchpadView, tr("Scratchpad"), QStringLiteral("Scratchpad"));

    auto* entitiesListView = new QListView(this);
    entitiesListView->setModel(_entitiesWithIconsModel);
    entitiesListView->setViewMode(QListView::ListMode);
    entitiesListView->setTextElideMode(Qt::ElideRight);
    entitiesListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    entitiesListView->setDragEnabled(true);
    auto* entitiesListDock = createDockWidget(entitiesListView, tr("Entities"), QStringLiteral("EntitiesList"));

    // Simplifies MainWindow state
    _roomEntitiesDock->setObjectName(QStringLiteral("RoomEntities"));

    this->addDockWidget(Qt::RightDockWidgetArea, propertyDock);
    this->addDockWidget(Qt::RightDockWidgetArea, tilesetDock);
    this->addDockWidget(Qt::RightDockWidgetArea, minimapDock);
    this->addDockWidget(Qt::RightDockWidgetArea, scratchpadDock);
    this->addDockWidget(Qt::RightDockWidgetArea, entitiesListDock);
    this->addDockWidget(Qt::RightDockWidgetArea, _roomEntitiesDock);

    this->tabifyDockWidget(minimapDock, scratchpadDock);
    scratchpadDock->raise();

    this->resizeDocks({ propertyDock, tilesetDock, scratchpadDock, entitiesListDock, _roomEntitiesDock }, { 100, 200, 200, 100, 100 }, Qt::Vertical);

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
