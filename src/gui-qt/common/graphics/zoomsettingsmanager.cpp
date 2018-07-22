/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "zoomsettingsmanager.h"
#include "zoomsettings.h"

using namespace UnTech::GuiQt;

ZoomSettingsManager::ZoomSettingsManager(QObject* parent)
    : QObject(parent)
    , _map()
    , _current(nullptr)
{
}

ZoomSettings* ZoomSettingsManager::get(const QString& name)
{
    ZoomSettings* z = _map.value(name, nullptr);

    if (z == nullptr) {
        z = new ZoomSettings(this);
        _map.insert(name, z);
    }

    return z;
}

void ZoomSettingsManager::set(const QString& name, qreal zoom, ZoomSettings::AspectRatio aspect)
{
    ZoomSettings* z = get(name);
    z->setZoom(zoom);
    z->setAspectRatio(aspect);
}
