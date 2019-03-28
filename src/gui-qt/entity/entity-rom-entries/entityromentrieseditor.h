/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "entityromentriesresourceitem.h"
#include "entityromentrylistwidget.h"
#include "entityromentrywidget.h"
#include "gui-qt/abstracteditor.h"

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityRomEntries {

class EntityRomEntriesEditor : public AbstractEditor {
    Q_OBJECT

public:
    EntityRomEntriesEditor(QWidget* parent)
        : AbstractEditor(parent)
        , _editorWidget(new EntityRomEntryWidget(parent))
        , _propertyWidget(new EntityRomEntryListWidget(parent))
    {
    }
    ~EntityRomEntriesEditor() = default;

    virtual bool setResourceItem(Project*, AbstractResourceItem* aItem)
    {
        auto* item = qobject_cast<EntityRomEntriesResourceItem*>(aItem);

        _editorWidget->setResourceItem(item);
        _propertyWidget->setResourceItem(item);

        return item != nullptr;
    }

    virtual QWidget* editorWidget() const final { return _editorWidget; }
    virtual QWidget* propertyWidget() const final { return _propertyWidget; }

private:
    EntityRomEntryWidget* const _editorWidget;
    EntityRomEntryListWidget* const _propertyWidget;
};

}
}
}
}
