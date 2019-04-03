/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourcelist.h"
#include "models/common/externalfilelist.h"
#include "models/metasprite/frameset-exportorder.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ExportOrder {

class ResourceList : public AbstractResourceList {
    Q_OBJECT

public:
    ResourceList(Project* project);
    ~ResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QList<AddResourceSettings>& addResourceSettings() const final;

protected:
    virtual size_t nItems() const final;
    virtual AbstractResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& filename) final;
    virtual void do_removeResource(unsigned index) final;

    // Will always return the same instance
    friend class ResourceItem;
    ExternalFileList<UnTech::MetaSprite::FrameSetExportOrder>& exportOrders() const;
};
}
}
}
}
