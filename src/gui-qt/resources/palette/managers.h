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
namespace Palette {

class PaletteResourceItem;

class PalettePropertyManager : public PropertyListManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        IMAGE_FILENAME,
        ROWS_PER_FRAME,
        ANIMATION_DELAY,
        SKIP_FIRST_FRAME,
    };

public:
    explicit PalettePropertyManager(QObject* parent = nullptr);
    ~PalettePropertyManager() = default;

    void setResourceItem(PaletteResourceItem* item);

    virtual void updateParameters(int id, QVariant& param1, QVariant& param2) const final;

    virtual QVariant data(int id) const final;
    virtual bool setData(int id, const QVariant& value) final;

private:
    PaletteResourceItem* _palette;
};
}
}
}
}
