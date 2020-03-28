/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/staticresourcelist.h"
#include "models/metasprite/common.h"

namespace UnTech {
namespace GuiQt {
class StaticResourceList;

namespace MetaSprite {
namespace ActionPoints {
class ActionPointFunctionsList;

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

public:
    ResourceItem(StaticResourceList* list, unsigned index);
    ~ResourceItem() = default;

    StaticResourceList* resourceList() const { return static_cast<StaticResourceList*>(_list); }

    ActionPointFunctionsList* actionPointFunctionsList() const { return _actionPointFunctionsList; }

    const UnTech::MetaSprite::ActionPointMapping& actionPointMapping() const { return _actionPointMapping; }

    QStringList actionPointNames() const;

protected:
    virtual bool compileResource(ErrorList& err) final;

private:
    ActionPointFunctionsList* const _actionPointFunctionsList;

    UnTech::MetaSprite::ActionPointMapping _actionPointMapping;
};

}
}
}
}
