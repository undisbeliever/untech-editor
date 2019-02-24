/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromstructlistwidget.h"
#include "entityromstructsresourceitem.h"
#include "entityromstructwidget.h"
#include "gui-qt/abstracteditor.h"

namespace UnTech {
namespace GuiQt {
namespace Entity {

class EntityRomStructsEditor : public AbstractEditor {
    Q_OBJECT

public:
    EntityRomStructsEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new EntityRomStructWidget(parent))
        , _propertyWidget(new EntityRomStructListWidget(parent))
    {
    }
    ~EntityRomStructsEditor() = default;

    virtual bool setResourceItem(Project*, AbstractResourceItem* aItem)
    {
        auto* item = qobject_cast<EntityRomStructsResourceItem*>(aItem);

        _editorWidget->setResourceItem(item);
        _propertyWidget->setResourceItem(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return _propertyWidget; }

private:
    EntityRomStructWidget* const _editorWidget;
    EntityRomStructListWidget* const _propertyWidget;
};

}
}
}
