/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertylistmanager.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {
class ResourceProject;

class ResourceFilePropertyManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        BLOCK_SIZE,
        BLOCK_COUNT,

        METATILE_MAX_MAP_SIZE,
        METATILE_N_METATILES,
    };

public:
    explicit ResourceFilePropertyManager(QObject* parent = nullptr);
    ~ResourceFilePropertyManager() = default;

    virtual void setProject(ResourceProject* project) final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    ResourceProject* _project;
};
}
}
}
