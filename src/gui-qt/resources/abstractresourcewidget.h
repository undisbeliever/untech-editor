/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "common.h"
#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class Document;
class AbstractResourceItem;

class AbstractResourceWidget : public QWidget {
    Q_OBJECT

public:
    AbstractResourceWidget(QWidget* parent)
        : QWidget(parent)
    {
    }
    ~AbstractResourceWidget() = default;

    virtual ResourceTypeIndex resourceTypeIndex() const = 0;

    // item MAY NOT be the type used by this widget, use qobject_cast to
    // determine if selected item is the right type.
    virtual void setResourceItem(AbstractResourceItem* item) = 0;
};
}
}
}
