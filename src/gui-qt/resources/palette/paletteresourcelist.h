/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "paletteresourceitem.h"
#include "gui-qt/abstractresourcelist.h"
#include "models/common/namedlist.h"
#include "models/resources/palette.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Palette {

class PaletteResourceList : public AbstractResourceList {
    Q_OBJECT

public:
    PaletteResourceList(Project* project);
    ~PaletteResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QList<AddResourceSettings>& addResourceSettings() const final;

protected:
    virtual size_t nItems() const final;
    virtual PaletteResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& inputString) final;
    virtual void do_removeResource(unsigned index) final;

    // Will always return the same instance
    friend class PaletteResourceItem;
    NamedList<RES::PaletteInput>& palettes() const;
};
}
}
}
}
