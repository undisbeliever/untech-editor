/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/abstractaccessors.h"
#include "models/metasprite/common.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace ActionPoints {
class ResourceItem;

using ActionPointFunction = UnTech::MetaSprite::ActionPointFunction;

class ActionPointFunctionsList : public Accessor::NamedListAccessor<ActionPointFunction, ResourceItem> {
    Q_OBJECT

public:
    ActionPointFunctionsList(ResourceItem* resourceItem);
    ~ActionPointFunctionsList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool edit_setManuallyInvoked(size_t index, bool manuallyInvoked);
};

}
}
}
}
