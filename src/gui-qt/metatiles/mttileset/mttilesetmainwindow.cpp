/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetmainwindow.h"
#include "mttilesetgraphicsscenes.h"
#include "mttilesetpropertymanager.h"
#include "mttilesetrenderer.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metatiles/mtgridgraphicsitem.h"
#include "gui-qt/metatiles/mttileset/mttilesetmainwindow.ui.h"
#include "gui-qt/metatiles/style.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/palette/paletteresourcelist.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

MtTilesetMainWindow::MtTilesetMainWindow(QWidget* parent, ZoomSettingsManager* zoomManager)
    : QMainWindow(parent)
    , _ui(new Ui::MtTilesetMainWindow)
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

    _renderer->setPlayButton(_ui->playButton);
    _renderer->setRegionCombo(_ui->region);

    _editableScratchpadScene->addGridSelectionSource(_scratchpadScene);
    _editableScratchpadScene->addGridSelectionSource(_tilesetScene);

    _ui->tilesetPropertyView->setPropertyManager(_tilesetPropertyManager);

    ZoomSettings* centralZoomSettings = zoomManager->get("metatiles");
    ZoomSettings* dockedZoomSettings = zoomManager->get("metatiles-dock");

    _ui->animationFramesInputWidget->setZoomSettings(centralZoomSettings);
    _ui->centralTilesetGraphicsView->setZoomSettings(centralZoomSettings);
    _ui->centralScratchpadGraphicsView->setZoomSettings(centralZoomSettings);
    _ui->dockedTilesetGraphicsView->setZoomSettings(dockedZoomSettings);
    _ui->dockedScratchpadGraphicsView->setZoomSettings(dockedZoomSettings);

    _ui->centralTilesetGraphicsView->setScene(_tilesetScene);
    _ui->centralScratchpadGraphicsView->setScene(_editableScratchpadScene);

    _ui->dockedTilesetGraphicsView->setScene(_tilesetScene);
    _ui->dockedScratchpadGraphicsView->setScene(_scratchpadScene);

    tabifyDockWidget(_ui->minimapDock, _ui->scratchpadDock);
    resizeDocks({ _ui->minimapDock }, { 1 }, Qt::Vertical);
    _ui->minimapDock->raise();

    connect(_ui->resetAnimationButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::resetAnimations);
    connect(_ui->nextTilesetFrameButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::pauseAndAdvanceTilesetFrame);
    connect(_ui->nextPaletteFrameButton, &QToolButton::clicked,
            _renderer, &MtTilesetRenderer::pauseAndAdvancePaletteFrame);

    connect(_ui->palette, qOverload<const QString&>(&QComboBox::activated),
            this, &MtTilesetMainWindow::onPaletteComboActivated);
}

MtTilesetMainWindow::~MtTilesetMainWindow() = default;

ZoomSettings* MtTilesetMainWindow::zoomSettings() const
{
    return _ui->centralTilesetGraphicsView->zoomSettings();
}

void MtTilesetMainWindow::populateMenu(QMenu* editMenu, QMenu* viewMenu)
{
    Q_UNUSED(editMenu);

    viewMenu->addSeparator();
    viewMenu->addAction(_style->showGridAction());
}

void MtTilesetMainWindow::setResourceItem(MtTilesetResourceItem* item)
{
    if (_tileset == item) {
        return;
    }

    if (_tileset) {
        _tileset->disconnect(this);
    }
    _tileset = item;

    _renderer->setTilesetItem(item);

    _tilesetPropertyManager->setResourceItem(item);
    _ui->animationFramesInputWidget->setResourceItem(item);

    onTilesetPalettesChanged();

    _editableScratchpadScene->createTileCursor();

    if (_tileset) {
        onTilesetStateChanged();
        connect(_tileset, &MtTilesetResourceItem::stateChanged,
                this, &MtTilesetMainWindow::onTilesetStateChanged);
        connect(_tileset, &MtTilesetResourceItem::palettesChanged,
                this, &MtTilesetMainWindow::onTilesetPalettesChanged);
    }
    else {
        _ui->playButton->setChecked(false);
    }

    setEnabled(item != nullptr);
}

void MtTilesetMainWindow::onTilesetStateChanged()
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

void MtTilesetMainWindow::onTilesetPalettesChanged()
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

void MtTilesetMainWindow::onPaletteComboActivated(const QString& paletteId)
{
    if (_tileset == nullptr) {
        return;
    }

    auto* pal = qobject_cast<Resources::PaletteResourceItem*>(
        _tileset->project()->paletteResourceList()->findResource(paletteId));

    _renderer->setPaletteItem(pal);
}

void MtTilesetMainWindow::onErrorDoubleClicked(const UnTech::ErrorListItem& error)
{
    if (_tileset == nullptr) {
        return;
    }

    _ui->animationFramesInputWidget->onErrorDoubleClicked(error);

    // The scratchpad can never generate an error
    _ui->tabWidget->setCurrentWidget(_ui->animationFramesInputWidget);
}
