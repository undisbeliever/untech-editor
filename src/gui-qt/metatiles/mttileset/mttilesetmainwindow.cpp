/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetmainwindow.h"
#include "mttilesetpropertymanager.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/metatiles/mttileset/mttilesetmainwindow.ui.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaTiles;

MtTilesetMainWindow::MtTilesetMainWindow(QWidget* parent, ZoomSettingsManager* zoomManager)
    : QMainWindow(parent)
    , _ui(new Ui::MtTilesetMainWindow)
    , _propertyManager(new MtTilesetPropertyManager(this))
    , _tileset(nullptr)
{
    Q_ASSERT(zoomManager);

    _ui->setupUi(this);

    _ui->propertyView->setPropertyManager(_propertyManager);

    ZoomSettings* centralZoomSettings = zoomManager->get("metatiles");
    ZoomSettings* dockedZoomSettings = zoomManager->get("metatiles-dock");

    _ui->animationFramesInputWidget->setZoomSettings(centralZoomSettings);
    _ui->centralTilesetGraphicsView->setZoomSettings(centralZoomSettings);
    _ui->centralScratchpadGraphicsView->setZoomSettings(centralZoomSettings);
    _ui->dockedTilesetGraphicsView->setZoomSettings(dockedZoomSettings);
    _ui->dockedScratchpadGraphicsView->setZoomSettings(dockedZoomSettings);

    tabifyDockWidget(_ui->minimapDock, _ui->scratchpadDock);
    resizeDocks({ _ui->minimapDock }, { 1 }, Qt::Vertical);
    _ui->minimapDock->raise();
}

MtTilesetMainWindow::~MtTilesetMainWindow() = default;

ZoomSettings* MtTilesetMainWindow::zoomSettings() const
{
    return _ui->centralTilesetGraphicsView->zoomSettings();
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

    _propertyManager->setResourceItem(item);
    _ui->animationFramesInputWidget->setResourceItem(item);

    if (_tileset) {
        onTilesetStateChanged();
        connect(_tileset, &MtTilesetResourceItem::stateChanged,
                this, &MtTilesetMainWindow::onTilesetStateChanged);
    }

    setEnabled(item != nullptr);
}

void MtTilesetMainWindow::onTilesetStateChanged()
{
    // ::TODO only show ANIMATION_FRAMES_INPUT_WIDGET when there is an error in the animationFramesInput::

    if (_tileset == nullptr) {
        _ui->stackedWidget->setCurrentIndex(StackIndex::ANIMATION_FRAMES_INPUT_WIDGET);
        return;
    }

    if (_tileset->state() != ResourceState::VALID) {
        _ui->stackedWidget->setCurrentIndex(StackIndex::ANIMATION_FRAMES_INPUT_WIDGET);
    }
    else {
        _ui->stackedWidget->setCurrentIndex(StackIndex::METATILES_CENTRAL_WIDGET);
    }
}
