/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertymanager.h"
#include "propertymodel.h"

using namespace UnTech::GuiQt;

PropertyManager::PropertyManager(QObject* parent)
    : QObject(parent)
{
}

void PropertyManager::addProperty(const QString& title, int id)
{
    _properties.append({ title, id });

    emit propertyListChanged();
}

void PropertyManager::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;
        emit enabledChanged();
    }
}
