/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertylistmanager.h"
#include "propertylistmodel.h"

using namespace UnTech::GuiQt;

PropertyListManager::PropertyListManager(QObject* parent)
    : QObject(parent)
    , _enabled(false)
{
}

const QString& PropertyListManager::propertyTitle(int id)
{
    static const QString nullString;

    for (const Property& p : _properties) {
        if (p.id == id) {
            return p.title;
        }
    }

    return nullString;
}

void PropertyListManager::addProperty(const QString& title, int id, PropertyType type,
                                      const QVariant& param1, const QVariant& param2)
{
    if (id < 0) {
        qWarning("PropertyListManager::addProperty: id must be >= 0");
    }

    _properties.append(Property(title, id, type, param1, param2));

    emit propertyListChanged();
}

void PropertyListManager::addPropertyGroup(const QString& title)
{
    _properties.append(Property(title, -1, PropertyType::STRING));

    emit propertyListChanged();
}

void PropertyListManager::setPropertyList(const QVector<Property> properties)
{
    _properties = properties;
    emit propertyListChanged();
}

void PropertyListManager::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;
        emit enabledChanged();
    }
}

void PropertyListManager::updateParameters(int, QVariant&, QVariant&) const
{
    // This function does not change the parameters by default
}
