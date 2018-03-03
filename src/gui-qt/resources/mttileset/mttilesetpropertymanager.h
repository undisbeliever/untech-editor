/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../genericpropertieswidget.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {

class MtTilesetResourceItem;

class MtTilesetPropertiesManager : public AbstractPropertyManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        FRAME_IMAGES,
        PALETTES,
        ANIMATION_DELAY,
        BIT_DEPTH,
        ADD_TRANSPARENT_TILE,
    };

public:
    explicit MtTilesetPropertiesManager(QObject* parent = nullptr);
    ~MtTilesetPropertiesManager() = default;

    virtual ResourceTypeIndex resourceTypeIndex() const final;
    virtual void setResourceItem(AbstractResourceItem* item) final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    MtTilesetResourceItem* _tileset;
};
}
}
}