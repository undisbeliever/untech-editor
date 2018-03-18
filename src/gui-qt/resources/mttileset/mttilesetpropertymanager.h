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

class MtTilesetPropertyManager : public AbstractPropertyManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        PALETTES,
        FRAME_IMAGES,
        ANIMATION_DELAY,
        BIT_DEPTH,
        ADD_TRANSPARENT_TILE,
    };

public:
    explicit MtTilesetPropertyManager(QObject* parent = nullptr);
    ~MtTilesetPropertyManager() = default;

    virtual void setResourceItem(AbstractResourceItem* item) final;

    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    MtTilesetResourceItem* _tileset;
};
}
}
}
