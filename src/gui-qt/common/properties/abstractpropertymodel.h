/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "property.h"
#include <QAbstractItemModel>

namespace UnTech {
namespace GuiQt {

class AbstractPropertyModel : public QAbstractItemModel {
    Q_OBJECT

public:
    static const Property blankProperty;

public:
    AbstractPropertyModel(QObject* parent)
        : QAbstractItemModel(parent)
    {
    }

    // Must return a blankProperty if index is a property title
    virtual const Property& propertyForIndex(const QModelIndex& index) const = 0;

    virtual QPair<QVariant, QVariant> propertyParametersForIndex(const QModelIndex& index) const = 0;

    virtual bool isListItem(const QModelIndex& index) const = 0;

protected:
    QString displayForProperty(const QModelIndex& index, const QVariant& value) const;
};
}
}
