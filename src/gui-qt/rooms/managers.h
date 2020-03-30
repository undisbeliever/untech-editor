/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertylistmanager.h"

namespace UnTech {
namespace GuiQt {
namespace Rooms {

class ResourceItem;

class RoomPropertyManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        SCENE,
        MAP_SIZE,
    };

public:
    explicit RoomPropertyManager(QObject* parent = nullptr);
    ~RoomPropertyManager() = default;

    void setResourceItem(ResourceItem* item);

    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    ResourceItem* _resourceItem;
};
}
}
}
