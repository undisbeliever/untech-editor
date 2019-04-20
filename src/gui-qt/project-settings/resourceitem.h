/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/project.h"
#include "gui-qt/staticresourcelist.h"
#include "models/common/idstring.h"

namespace UnTech {
namespace GuiQt {
class StaticResourceList;

namespace ProjectSettings {

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

public:
    using DataT = UnTech::Project::ProjectFile;

    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

public:
    ResourceItem(StaticResourceList* list, unsigned index);
    ~ResourceItem() = default;

    bool editBlockSettings_setSize(unsigned blockSize);
    bool editBlockSettings_setCount(unsigned blockCount);

    bool editMetaTileSettings_setMaxMapSize(unsigned maxMapSize);
    bool editMetaTileSettings_setNMetaTiles(unsigned nMetaTiles);

    bool editEntityRomData_setEntityListIds(const std::vector<idstring>& listIds);

protected:
    friend class Accessor::ResourceItemUndoHelper<ResourceItem>;
    DataT* dataEditable() const { return project()->projectFile(); }

signals:
    void entityListIdsChanged();

protected:
    virtual bool compileResource(ErrorList& err) final;
};

}
}
}
