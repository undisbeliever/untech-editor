/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "mttilesetresourceitem.h"
#include "../abstractresourcelist.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Resources {

class MtTilesetResourceList : public AbstractResourceList {
    Q_OBJECT

public:
    MtTilesetResourceList(ResourceProject* project, ResourceTypeIndex typeIndex);
    ~MtTilesetResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;
    virtual const AddResourceDialogSettings& addResourceDialogSettings() const final;

protected:
    virtual size_t nItems() const final;
    virtual MtTilesetResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(const std::string& filename) final;
    virtual void do_removeResource(unsigned index) final;
};
}
}
}
