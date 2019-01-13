/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertylistmanager.h"

namespace UnTech {
namespace GuiQt {
class Project;

namespace Resources {

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

    virtual void setProject(Project* project) final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    bool editBlockSettings_setSize(unsigned blockSize);
    bool editBlockSettings_setCount(unsigned blockCount);

    bool editMetaTileSettings_setMaxMapSize(unsigned maxMapSize);
    bool editMetaTileSettings_setNMetaTiles(unsigned nMetaTiles);

private:
    Project* _project;
};
}
}
}
