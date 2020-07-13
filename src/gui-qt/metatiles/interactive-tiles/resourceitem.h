/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/staticresourcelist.h"

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
namespace InteractiveTiles {
class FunctionTableList;

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

public:
    ResourceItem(StaticResourceList* list, unsigned index);
    ~ResourceItem() = default;

    StaticResourceList* resourceList() const { return static_cast<StaticResourceList*>(_list); }

    FunctionTableList* functionTableList() const { return _functionTableList; }

protected:
    virtual bool compileResource(ErrorList& err) final;

private:
    FunctionTableList* const _functionTableList;
};

}
}
}
}
