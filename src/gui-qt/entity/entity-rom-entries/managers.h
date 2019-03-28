/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertylistmanager.h"
#include "gui-qt/entity/structfields.h"
#include <QList>

namespace UnTech {
namespace GuiQt {
namespace Entity {
namespace EntityRomEntries {
class EntityRomEntriesResourceItem;

class EntityRomEntryManager : public PropertyListManager {
    Q_OBJECT

    enum FixedFieldIds {
        INITIAL_PROJECTILE_ID,
        INTITAL_LIST_ID,
        FRAME_SET_ID,
        DEFAULT_PALETTE,
        DISPLAY_FRAME,
    };
    constexpr static int FIXED_FIELDS_SIZE = 5;

public:
    explicit EntityRomEntryManager(QObject* parent = nullptr);
    ~EntityRomEntryManager() = default;

    void setResourceItem(EntityRomEntriesResourceItem* item);

    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private slots:
    void onSelectedEntryChanged();
    void onImplementsChanged(size_t index);
    void onEntryDataChanged(size_t index);

    void rebuildStructFields();
    void rebuildEntryFields();

private:
    StructFields _structFields;
    QHash<QString, QString> _entryFields;

    EntityRomEntriesResourceItem* _item;
};

}
}
}
}
