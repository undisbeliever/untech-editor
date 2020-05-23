/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QString>
#include <QVariant>

namespace UnTech {
namespace GuiQt {

enum class PropertyType {
    BOOLEAN,       // no parameters
    INTEGER,       // minimum value, maximum value
    UNSIGNED,      // minimum value, maximum value
    UNSIGNED_HEX,  // minimum value, maximum value
    STRING,        // no parameters
    IDSTRING,      // completer stringlist
    FILENAME,      // dialog filter
    COLOR,         // no parameters
    POINT,         // minimum value, maximum value
    SIZE,          // minimum value, maximum value
    RECT,          // bounding rect, maximum size
    COMBO,         // StringList of values, (optional) data VariantList
    STRING_COMBO,  // StringList of values. Faster than COMBO, parameter only accessed by setEditorData.
    COLOR_COMBO,   // QVariantList of colors. Faster than COMBO, parameter only accessed by setEditorData.
    STRING_LIST,   // no parameters
    IDSTRING_LIST, // completer values
    FILENAME_LIST, // dialog filter

    NOT_EDITABLE, // property is not editable (ie. status information)
};

inline bool isListType(PropertyType& type)
{
    return type == PropertyType::STRING_LIST
           || type == PropertyType::IDSTRING_LIST
           || type == PropertyType::FILENAME_LIST;
}

struct Property {
    const QString title;
    const int id;
    const PropertyType type;

    const QVariant parameter1;
    const QVariant parameter2;

    const bool isList;
    const bool isEditable;

    Property(const QString& title, int id, PropertyType type,
             const QVariant& param1 = QVariant(), const QVariant& param2 = QVariant())
        : title(title)
        , id(id)
        , type(type)
        , parameter1(param1)
        , parameter2(param2)
        , isList(isListType(type))
        , isEditable(id >= 0 && type != PropertyType::NOT_EDITABLE)
    {
    }

    Property()
        : Property(QString(), -1, PropertyType::STRING)
    {
    }
};
}
}
