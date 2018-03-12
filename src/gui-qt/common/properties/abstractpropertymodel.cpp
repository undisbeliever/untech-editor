/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractpropertymodel.h"

#include <QPoint>
#include <QRect>
#include <QSize>

using namespace UnTech::GuiQt;
using Type = PropertyType;

const Property AbstractPropertyModel::blankProperty;

QString AbstractPropertyModel::displayForProperty(const QModelIndex& index, const QVariant& value) const
{
    const Property& settings = propertyForIndex(index);

    switch (settings.type) {
    case Type::BOOLEAN:
    case Type::INTEGER:
    case Type::UNSIGNED:
    case Type::STRING:
    case Type::IDSTRING:
    case Type::FILENAME:
    case Type::COLOR: {
        return value.toString();
    }

    case Type::POINT: {
        QPoint p = value.toPoint();
        return QString("%1, %2")
            .arg(p.x())
            .arg(p.y());
    }

    case Type::SIZE: {
        QSize s = value.toSize();
        return QString("%1 x %2")
            .arg(s.width())
            .arg(s.height());
    }

    case Type::RECT: {
        QRect r = value.toRect();
        return QString("%1, %2 : %3 x %4")
            .arg(r.x())
            .arg(r.y())
            .arg(r.width())
            .arg(r.height());
    }

    case Type::COMBO: {
        auto params = propertyParametersForIndex(index);

        int index = -1;
        if (params.second.canConvert(QVariant::List)) {
            index = params.second.toList().indexOf(value);
        }

        QStringList displayList = params.first.toStringList();
        if (index >= 0 && index < displayList.size()) {
            return displayList.at(index);
        }
        else {
            return value.toString();
        }
    }

    case Type::STRING_LIST:
    case Type::IDSTRING_LIST:
    case Type::FILENAME_LIST: {
        int listSize = 0;

        if (value.type() == QVariant::StringList) {
            listSize = value.toStringList().size();
        }
        else if (value.type() == QVariant::List) {
            listSize = value.toList().size();
        }

        if (listSize != 1) {
            return tr("(%1 items)").arg(listSize);
        }
        else {
            return tr("(1 item)");
        }
    }
    }

    return QString();
}
