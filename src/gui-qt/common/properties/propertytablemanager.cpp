/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertytablemanager.h"
#include "propertytablemodel.h"

using namespace UnTech::GuiQt;

PropertyTableManager::PropertyTableManager(QObject* parent)
    : QObject(parent)
{
}

QStringList PropertyTableManager::propertyTitles() const
{
    QStringList sl;
    sl.reserve(_properties.size());
    std::transform(_properties.cbegin(), _properties.cend(), std::back_inserter(sl),
                   [](const Property& p) { return p.title; });
    return sl;
}

const QString& PropertyTableManager::propertyTitle(int id)
{
    static const QString nullString;

    for (const Property& p : _properties) {
        if (p.id == id) {
            return p.title;
        }
    }

    return nullString;
}

void PropertyTableManager::addProperty(const QString& title, int id, PropertyType type,
                                       const QVariant& param1, const QVariant& param2)
{
    _properties.append(Property(title, id, type, param1, param2));

    emit propertyListChanged();
}

void PropertyTableManager::setTitle(const QString& title)
{
    if (_title != title) {
        _title = title;
        emit titleChanged();
    }
}

void PropertyTableManager::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;
        emit enabledChanged();
    }
}

void PropertyTableManager::updateParameters(int, int, QVariant&, QVariant&) const
{
}

bool PropertyTableManager::canInsertItem()
{
    return false;
}

bool PropertyTableManager::canCloneItem(int)
{
    return false;
}

bool PropertyTableManager::insertItem(int)
{
    return false;
}

bool PropertyTableManager::cloneItem(int)
{
    return false;
}

bool PropertyTableManager::removeItem(int)
{
    return false;
}
