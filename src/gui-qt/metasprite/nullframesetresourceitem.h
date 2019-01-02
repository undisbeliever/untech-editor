/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "metaspriteproject.h"
#include "gui-qt/abstractresourceitem.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class FrameSetResourceList;

class NullFrameSetResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

public:
    NullFrameSetResourceItem(FrameSetResourceList* parent, size_t index);
    ~NullFrameSetResourceItem() = default;

    MetaSpriteProject* project() const { return static_cast<MetaSpriteProject*>(_project); }

protected:
    virtual bool compileResource(ErrorList&) final;
};
}
}
}
