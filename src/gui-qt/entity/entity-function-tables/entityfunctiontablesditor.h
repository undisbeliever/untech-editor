/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityfunctiontablesresourceitem.h"
#include "entityfunctiontableswidget.h"
#include "gui-qt/abstracteditor.h"

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityFunctionTables {

class EntityFunctionTablesEditor : public AbstractEditor {
    Q_OBJECT

public:
    EntityFunctionTablesEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new EntityFunctionTablesWidget(parent))
    {
    }
    ~EntityFunctionTablesEditor() = default;

    virtual bool setResourceItem(Project*, AbstractResourceItem* aItem)
    {
        auto* item = qobject_cast<EntityFunctionTablesResourceItem*>(aItem);

        _editorWidget->setResourceItem(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return nullptr; }

private:
    EntityFunctionTablesWidget* const _editorWidget;
};

}
}
}
}
