/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/metasprite/abstractmsresourceitem.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationsList : public Accessor::NamedListAccessor<MSA::Animation, AbstractMsResourceItem> {
    Q_OBJECT

public:
    AnimationsList(AbstractMsResourceItem* resourceItem);
    ~AnimationsList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelected_setDurationFormat(MSA::DurationFormat durationFormat);
    bool editSelected_setOneShot(bool oneShot);

    // Also sets oneShot to false
    bool editSelected_setNextAnimation(const idstring& nextAnimation);
};

class AnimationFramesList : public Accessor::ChildVectorMultipleSelectionAccessor<MSA::AnimationFrame, AbstractMsResourceItem> {
    Q_OBJECT

public:
    AnimationFramesList(AbstractMsResourceItem* resourceItem);
    ~AnimationFramesList() = default;

    virtual QString typeName() const final;
    virtual QString typeNamePlural() const final;

    bool editSelectedList_setData(index_type index, const DataT& value);
};
}
}
}
}
