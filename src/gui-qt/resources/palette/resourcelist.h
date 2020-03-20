/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/abstractresourcelist.h"
#include "models/common/namedlist.h"
#include "models/resources/palette.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Palette {
class ResourceItem;

class ResourceList : public AbstractResourceList {
    Q_OBJECT

public:
    ResourceList(Project* project);
    ~ResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QVector<AddResourceSettings>& addResourceSettings() const final;

    ResourceItem* findResource(const QString& name) const;

protected:
    virtual size_t nItems() const final;
    virtual ResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& inputString) final;
    virtual void do_removeResource(unsigned index) final;

    // Will always return the same instance
    friend class ResourceItem;
    NamedList<RES::PaletteInput>& palettes() const;
};
}
}
}
}
