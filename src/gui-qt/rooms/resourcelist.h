/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "gui-qt/abstractresourcelist.h"
#include "models/rooms/rooms.h"
#include <QObject>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Rooms {

class ResourceList final : public AbstractResourceList {
    Q_OBJECT

public:
    ResourceList(Project* project);
    ~ResourceList() = default;

    virtual const QString resourceTypeNameSingle() const final;
    virtual const QString resourceTypeNamePlural() const final;

    virtual const QVector<AddResourceSettings>& addResourceSettings() const final;

    ResourceItem* findResource(const QString& name) const
    {
        return qobject_cast<ResourceItem*>(AbstractResourceList::findResource(name));
    }

protected:
    virtual size_t nItems() const final;
    virtual ResourceItem* buildResourceItem(size_t index) final;

    virtual void do_addResource(int settingIndex, const std::string& inputString) final;
    virtual void do_removeResource(unsigned index) final;

private:
    // Will always return the same instance
    friend class ResourceItem;
    ExternalFileList<UnTech::Rooms::RoomInput>& rooms() const;
};
}
}
}
