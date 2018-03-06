/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertymanager.h"
#include "propertymodel.h"

using namespace UnTech::GuiQt;

const PropertyManager::Property PropertyManager::blankProperty = {
    QString(), -1, PropertyManager::Type::STRING, false
};

PropertyManager::PropertyManager(QObject* parent)
    : QObject(parent)
{
}

const QString& PropertyManager::propertyTitle(int id)
{
    static const QString nullString;

    for (const Property& p : _properties) {
        if (p.id == id) {
            return p.title;
        }
    }

    return nullString;
}

void PropertyManager::addProperty(const QString& title, int id, Type type)
{
    bool isList = type == Type::STRING_LIST
                  || type == Type::IDSTRING_LIST
                  || type == Type::FILENAME_LIST;

    _properties.append({ title, id, type, isList });

    emit propertyListChanged();
}

void PropertyManager::addSeperator(const QString& title)
{
    addProperty(title, -1, Type::STRING);
}

void PropertyManager::setEnabled(bool enabled)
{
    if (_enabled != enabled) {
        _enabled = enabled;
        emit enabledChanged();
    }
}
