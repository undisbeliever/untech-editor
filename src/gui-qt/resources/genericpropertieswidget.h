/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractresourcewidget.h"
#include "gui-qt/common/properties/propertylistmanager.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Ui {
class GenericPropertiesWidget;
}
class PaletteResourceItem;

class AbstractPropertyManager : public PropertyListManager {
    Q_OBJECT

public:
    AbstractPropertyManager(QObject* parent = nullptr)
        : PropertyListManager(parent)
    {
    }

    virtual ResourceTypeIndex resourceTypeIndex() const = 0;
    virtual void setResourceItem(AbstractResourceItem* item) = 0;
};

class GenericPropertiesWidget : public AbstractResourceWidget {
    Q_OBJECT

public:
    GenericPropertiesWidget(AbstractPropertyManager* manager, QWidget* parent = 0);
    ~GenericPropertiesWidget();

    virtual ResourceTypeIndex resourceTypeIndex() const final;
    virtual void setResourceItem(AbstractResourceItem* item) final;

private:
    std::unique_ptr<Ui::GenericPropertiesWidget> _ui;
    AbstractPropertyManager* const _manager;
};
}
}
}
