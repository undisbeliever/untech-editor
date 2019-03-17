/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertylistmanager.h"

namespace UnTech {
namespace GuiQt {
namespace ProjectSettings {
class ProjectSettingsResourceItem;

class ProjectSettingsPropertyManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        BLOCK_SIZE,
        BLOCK_COUNT,

        METATILE_MAX_MAP_SIZE,
        METATILE_N_METATILES,

        ENTITY_LIST_IDS,
    };

public:
    explicit ProjectSettingsPropertyManager(QObject* parent = nullptr);
    ~ProjectSettingsPropertyManager() = default;

    virtual void setResourceItem(ProjectSettingsResourceItem* item) final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    ProjectSettingsResourceItem* _item;
};
}
}
}
