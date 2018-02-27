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

class PaletteResourceItem;

class PalettePropertiesManager : public AbstractPropertyManager {
    Q_OBJECT

    enum PropertyId {
        NAME,
        IMAGE_FILENAME,
        ROWS_PER_FRAME,
        ANIMATION_DELAY,
        SKIP_FIRST_FRAME,
    };

public:
    explicit PalettePropertiesManager(QObject* parent = nullptr);
    ~PalettePropertiesManager() = default;

    virtual ResourceTypeIndex resourceTypeIndex() const final;
    virtual void setResourceItem(AbstractResourceItem* item) final;

    virtual QVariant data(int id) const final;

private:
    PaletteResourceItem* _palette;
};
}
}
}
