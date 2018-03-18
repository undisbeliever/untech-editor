/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertylistmanager.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Ui {
class GenericPropertiesWidget;
}
class AbstractResourceItem;

class AbstractPropertyManager : public PropertyListManager {
    Q_OBJECT

public:
    AbstractPropertyManager(QObject* parent = nullptr)
        : PropertyListManager(parent)
    {
    }

    virtual void setResourceItem(AbstractResourceItem* item) = 0;
};

class GenericPropertiesWidget : public QWidget {
    Q_OBJECT

public:
    GenericPropertiesWidget(AbstractPropertyManager* manager, QWidget* parent = 0);
    ~GenericPropertiesWidget();

    void setResourceItem(AbstractResourceItem* item);

private:
    std::unique_ptr<Ui::GenericPropertiesWidget> _ui;
    AbstractPropertyManager* const _manager;
};
}
}
}
