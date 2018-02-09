/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../abstractresourceitem.h"
#include "../document.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {

class PaletteResourceItem : public AbstractResourceItem {
    Q_OBJECT

public:
    PaletteResourceItem(AbstractResourceList* parent, size_t index);
    ~PaletteResourceItem() = default;

    virtual const QString name() const final;
    virtual const QString filename() const final;

private:
    inline const auto& palettesData() const
    {
        return document()->resourcesFile()->palettes;
    }
};
}
}
}
