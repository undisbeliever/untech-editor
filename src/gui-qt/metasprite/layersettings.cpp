/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "layersettings.h"

using namespace UnTech::GuiQt::MetaSprite;

LayerSettings::LayerSettings(QObject* parent)
    : QObject(parent)
{
    _showAllAction = new QAction(tr("Show &All Layers"), this);

    _showOriginAction = new QAction(tr("O&rigin"), this);
    _showOriginAction->setCheckable(true);
    _showOriginAction->setShortcut(Qt::ALT + Qt::Key_G);

    _showTileHitboxAction = new QAction(tr("&Tile Hitbox"), this);
    _showTileHitboxAction->setCheckable(true);
    _showTileHitboxAction->setShortcut(Qt::ALT + Qt::Key_H);

    _showFrameObjectsAction = new QAction(tr("Frame &Objects"), this);
    _showFrameObjectsAction->setCheckable(true);
    _showFrameObjectsAction->setShortcut(Qt::ALT + Qt::Key_J);

    _showActionPointsAction = new QAction(tr("Action &Points"), this);
    _showActionPointsAction->setCheckable(true);
    _showActionPointsAction->setShortcut(Qt::ALT + Qt::Key_K);

    _showEntityHitboxesAction = new QAction(tr("Entity &Hitboxes"), this);
    _showEntityHitboxesAction->setCheckable(true);
    _showEntityHitboxesAction->setShortcut(Qt::ALT + Qt::Key_L);

    showAll();

    connect(_showAllAction, &QAction::triggered, this, &LayerSettings::showAll);

    connect(_showOriginAction, &QAction::toggled, this, &LayerSettings::setShowOrigin);
    connect(_showTileHitboxAction, &QAction::toggled, this, &LayerSettings::setShowTileHitbox);
    connect(_showFrameObjectsAction, &QAction::toggled, this, &LayerSettings::setShowFrameObjects);
    connect(_showActionPointsAction, &QAction::toggled, this, &LayerSettings::setShowActionPoints);
    connect(_showEntityHitboxesAction, &QAction::toggled, this, &LayerSettings::setShowEntityHitboxes);
}

void LayerSettings::populateMenu(QMenu* menu)
{
    menu->addAction(_showAllAction);
    menu->addAction(_showOriginAction);
    menu->addAction(_showTileHitboxAction);
    menu->addAction(_showFrameObjectsAction);
    menu->addAction(_showActionPointsAction);
    menu->addAction(_showEntityHitboxesAction);
}

void LayerSettings::showAll()
{
    _showOrigin = true;
    _showTileHitbox = true;
    _showFrameObjects = true;
    _showActionPoints = true;
    _showEntityHitboxes = true;

    _showOriginAction->setChecked(true);
    _showTileHitboxAction->setChecked(true);
    _showFrameObjectsAction->setChecked(true);
    _showActionPointsAction->setChecked(true);
    _showEntityHitboxesAction->setChecked(true);

    emit layerSettingsChanged();
}

void LayerSettings::setShowOrigin(bool showOrigin)
{
    _showOriginAction->setChecked(showOrigin);

    if (_showOrigin != showOrigin) {
        _showOrigin = showOrigin;
        emit layerSettingsChanged();
    }
}

void LayerSettings::setShowTileHitbox(bool showTileHitbox)
{
    _showTileHitboxAction->setChecked(showTileHitbox);

    if (_showTileHitbox != showTileHitbox) {
        _showTileHitbox = showTileHitbox;
        emit layerSettingsChanged();
    }
}

void LayerSettings::setShowFrameObjects(bool showFrameObjects)
{
    _showFrameObjectsAction->setChecked(showFrameObjects);

    if (_showFrameObjects != showFrameObjects) {
        _showFrameObjects = showFrameObjects;
        emit layerSettingsChanged();
    }
}

void LayerSettings::setShowActionPoints(bool showActionPoints)
{
    _showActionPointsAction->setChecked(showActionPoints);

    if (_showActionPoints != showActionPoints) {
        _showActionPoints = showActionPoints;
        emit layerSettingsChanged();
    }
}

void LayerSettings::setShowEntityHitboxes(bool showEntityHitboxes)
{
    _showEntityHitboxesAction->setChecked(showEntityHitboxes);

    if (_showEntityHitboxes != showEntityHitboxes) {
        _showEntityHitboxes = showEntityHitboxes;
        emit layerSettingsChanged();
    }
}
