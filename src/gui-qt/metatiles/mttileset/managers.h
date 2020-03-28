/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/genericpropertieswidget.h"

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
namespace MtTileset {

class ResourceItem;

class MtTilesetPropertyManager : public AbstractPropertyManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        SCRATCHPAD_SIZE,
        TILESET_PALETTES,
        FRAME_IMAGES,
        CONVERSION_PALETTE,
        ANIMATION_DELAY,
        BIT_DEPTH,
        ADD_TRANSPARENT_TILE,

        N_STATIC_TILES,
        N_ANIMATED_TILES,
    };

public:
    explicit MtTilesetPropertyManager(QObject* parent = nullptr);
    ~MtTilesetPropertyManager() = default;

    virtual void setResourceItem(AbstractResourceItem* item) final;

    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    ResourceItem* _tileset;
};
}
}
}
}
