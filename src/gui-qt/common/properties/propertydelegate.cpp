/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "propertydelegate.h"
#include "propertymanager.h"

using namespace UnTech::GuiQt;

PropertyDelegate::PropertyDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QString PropertyDelegate::displayText(const QVariant& value, const QLocale& locale) const
{
    auto itemCountString = [](int s) {
        if (s != 1) {
            return tr("(%1 items)").arg(s);
        }
        else {
            return tr("(1 item)");
        }
    };

    if (value.type() == QVariant::List) {
        return itemCountString(value.toList().size());
    }
    else if (value.type() == QVariant::StringList) {
        return itemCountString(value.toStringList().size());
    }
    else {
        return QStyledItemDelegate::displayText(value, locale);
    }
}
