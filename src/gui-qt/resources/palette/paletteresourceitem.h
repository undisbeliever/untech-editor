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

class PaletteResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

public:
    PaletteResourceItem(AbstractResourceList* parent, size_t index);
    ~PaletteResourceItem() = default;

    virtual QString name() const final;

protected:
    virtual bool compileResource(RES::ErrorList& err) final;

public:
    inline const auto& paletteData() const
    {
        return document()->resourcesFile()->palettes.at(index());
    }

private:
    inline const auto& palettesData() const
    {
        return document()->resourcesFile()->palettes;
    }
};
}
}
}
