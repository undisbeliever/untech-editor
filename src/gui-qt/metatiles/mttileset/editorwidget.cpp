/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "managers.h"
#include "mttilesetgraphicsscenes.h"
#include "mttilesetrenderer.h"
#include "resourceitem.h"
#include "tilepropertieswidget.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/common/properties/propertylistview.h"
#include "gui-qt/metatiles/mtgridgraphicsitem.h"
#include "gui-qt/metatiles/mttileset/editorwidget.ui.h"
#include "gui-qt/metatiles/style.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/palette/resourcelist.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;
using namespace UnTech::GuiQt::MetaTiles::MtTileset;

EditorWidget::EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(new Ui::EditorWidget)
    , _propertyListView(new PropertyListView(this))
    , _tilePropertiesWidget(new TilePropertiesWidget(this))
    , _dockedTilesetView(new OpenGLZoomableGraphicsView(this))
    , _dockedScratchpadView(new OpenGLZoomableGraphicsView(this))
    , _style(new Style(this))
    , _tilesetPropertyManager(new MtTilesetPropertyManager(this))
    , _renderer(new MtTilesetRenderer(this))
    , _tilesetScene(new MtTilesetGraphicsScene(_style, _renderer, this))
    , _scratchpadScene(new MtScratchpadGraphicsScene(_style, _renderer, this))
    , _editableScratchpadScene(new MtEditableScratchpadGraphicsScene(_style, _renderer, this))
    , _tileset(nullptr)
{
    Q_ASSERT(zoomManager);

    _ui->setupUi(this);

    _editableScratchpadScene->populateActions(_ui->scratchpadToolBar);

    _renderer->setPlayButton(_ui->playButton);
    _renderer->setRegionCombo(_ui->region);

    _editableScratchpadScene->addGridSelectionSource(_scratchpadScene);
    _editableScratchpadScene->addGridSelectionSource(_tilesetScene);

    _propertyListView->setPropertyManager(_tilesetPropertyManager);

    ZoomSettings* centralZoomSettings = zoomManager->get("metatiles");
    ZoomSettings* dockedZoomSettings = zoomManager->get("metatiles-dock");

    _ui->animationFramesInputWidget->setZoomSettings(centralZoomSettings);
    _ui->centralTilesetGraphicsView->setZoomSettings(centralZoomSettings);
    _ui->centralScratchpadGraphicsView->setZoomSettings(centralZoomSettings);

    _dockedScratchpadView->setMinimumSize(256, 256);
    _dockedTilesetView->setMinimumSize(256, 256);

    _dockedScratchpadView->setZoomSettings(dockedZoomSettings);
    _dockedTilesetView->setZoomSettings(dockedZoomSettings);

    _ui->centralTilesetGraphicsView->setScene(_tilesetScene);
    _ui->centralScratchpadGraphicsView->setScene(_editableScratchpadScene);

    _dockedTilesetView->setScene(_tilesetScene);
    _dockedScratchpadView->setScene(_scratchpadScene);

    auto* tilesetPropertyDock = createDockWidget(_propertyListView, tr("Tileset Properties"), QStringLiteral("MtTileset_Properties"));
    auto* tilePropertiesDock = createDockWidget(_tilePropertiesWidget, tr("Tile Properties"), QStringLiteral("MtTileset_TileProperties"));
    auto* tilesetDock = createDockWidget(_dockedTilesetView, tr("MetaTiles"), QStringLiteral("MtTileset_MetaTiles"));
    auto* scratchpadDock = createDockWidget(_dockedScratchpadView, tr("Scratchpad"), QStringLiteral("MtTileset_Scratchpad"));

    this->addDockWidget(Qt::RightDockWidgetArea, tilesetPropertyDock);
    this->addDockWidget(Qt::RightDockWidgetArea, tilePropertiesDock);
    this->addDockWidget(Qt::RightDockWidgetArea, tilesetDock);
    this->addDockWidget(Qt::RightDockWidgetArea, scratchpadDock);

    // Tile and Tileset properties share a dock tab so I can easily hide the Tile Properties Dock when editing the scratchpad.
    this->tabifyDockWidget(tilesetPropertyDock, tilePropertiesDock);
    this->tabifyDockWidget(tilesetDock, scratchpadDock);
    tilesetDock->raise();

    this->resizeDocks({ tilesetPropertyDock, tilesetDock }, { 100, 200 }, Qt::Vertical);

    onTabChanged();

    connect(_ui->resetAnimationButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::resetAnimations);
    connect(_ui->nextTilesetFrameButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::pauseAndAdvanceTilesetFrame);
    connect(_ui->nextPaletteFrameButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::pauseAndAdvancePaletteFrame);

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    connect(_ui->palette, &QComboBox::textActivated,
            this, &EditorWidget::onPaletteComboActivated);
#else
    connect(_ui->palette, qOverload<const QString&>(&QComboBox::activated),
            this, &EditorWidget::onPaletteComboActivated);
#endif

    connect(_ui->tabWidget, &QTabWidget::currentChanged,
            this, &EditorWidget::onTabChanged);
}

EditorWidget::~EditorWidget() = default;

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("MtTileset");
}

ZoomSettings* EditorWidget::zoomSettings() const
{
    return _ui->centralTilesetGraphicsView->zoomSettings();
}

void EditorWidget::populateMenu(QMenu* editMenu, QMenu* viewMenu)
{
    editMenu->addSeparator();
    _editableScratchpadScene->populateActions(editMenu);

    viewMenu->addSeparator();
    _style->populateActions(viewMenu);
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);
    if (_tileset == item) {
        return item != nullptr;
    }

    if (_tileset) {
        _tileset->disconnect(this);
    }
    _tileset = item;

    _renderer->setTilesetItem(item);

    _tilesetPropertyManager->setResourceItem(item);
    _tilePropertiesWidget->setResourceItem(item);
    _ui->animationFramesInputWidget->setResourceItem(item);

    onTilesetPalettesChanged();

    if (_tileset) {
        onTilesetStateChanged();
        connect(_tileset, &ResourceItem::stateChanged,
                this, &EditorWidget::onTilesetStateChanged);
        connect(_tileset, &ResourceItem::palettesChanged,
                this, &EditorWidget::onTilesetPalettesChanged);
    }
    else {
        _ui->playButton->setChecked(false);
    }

    setEnabled(item != nullptr);

    return item != nullptr;
}

void EditorWidget::onTilesetStateChanged()
{
    // ::TODO only show ANIMATION_FRAMES_INPUT_WIDGET when there is an error in the animationFramesInput::

    if (_tileset == nullptr) {
        _ui->playButton->setChecked(false);
        _ui->stackedWidget->setCurrentIndex(StackIndex::ANIMATION_FRAMES_INPUT_WIDGET);
        return;
    }

    if (_tileset->state() == ResourceState::ERROR) {
        _ui->playButton->setChecked(false);
        _ui->stackedWidget->setCurrentIndex(StackIndex::ANIMATION_FRAMES_INPUT_WIDGET);
    }
    else if (_tileset->state() == ResourceState::VALID) {
        _ui->animationFramesInputWidget->stopAnimations();
        _ui->stackedWidget->setCurrentIndex(StackIndex::METATILES_CENTRAL_WIDGET);
    }
}

void EditorWidget::onTilesetPalettesChanged()
{
    QStringList pals;

    if (_tileset) {
        if (const auto* data = _tileset->data()) {
            pals = convertStringList(data->palettes);
        }
    }

    auto* currentPaletteItem = _renderer->paletteItem();
    const QString currentPaletteName = currentPaletteItem ? currentPaletteItem->name() : QString();

    _ui->palette->clear();
    _ui->palette->addItems(pals);

    int currentPaletteIndex = pals.indexOf(currentPaletteName);

    if (currentPaletteIndex > 0) {
        _ui->palette->setCurrentIndex(currentPaletteIndex);
    }
    else {
        if (pals.isEmpty()) {
            _renderer->setPaletteItem(nullptr);
        }
        else {
            _ui->palette->setCurrentIndex(0);
            onPaletteComboActivated(pals.first());
        }
    }
}

void EditorWidget::onPaletteComboActivated(const QString& paletteId)
{
    if (_tileset == nullptr) {
        return;
    }

    auto* pal = _tileset->project()->palettes()->findResource(paletteId);

    _renderer->setPaletteItem(pal);
}

void EditorWidget::onTabChanged()
{
    _ui->scratchpadToolBar->setEnabled(_ui->tabWidget->currentWidget() == _ui->centralScratchpadGraphicsView);
}

void EditorWidget::onErrorDoubleClicked(const UnTech::ErrorListItem& error)
{
    if (_tileset == nullptr) {
        return;
    }

    _ui->animationFramesInputWidget->onErrorDoubleClicked(error);

    // The scratchpad can never generate an error
    _ui->tabWidget->setCurrentWidget(_ui->animationFramesInputWidget);
}
