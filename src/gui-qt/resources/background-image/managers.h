/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/properties/propertylistmanager.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace BackgroundImage {

class ResourceItem;

class BackgroundImagePropertyManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        BIT_DEPTH,
        IMAGE_FILENAME,
        CONVERSION_PALETTE,
        FIRST_PALETTE,
        N_PALETTES,
        DEFAULT_ORDER,

        N_TILES,
    };

public:
    explicit BackgroundImagePropertyManager(QObject* parent = nullptr);
    ~BackgroundImagePropertyManager() = default;

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
}
