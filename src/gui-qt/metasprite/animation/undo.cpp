/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationaccessors.h"
#include "gui-qt/accessor/listundohelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::Animation;

using AnimationsListUndoHelper = ListAndSelectionUndoHelper<AnimationsList>;

bool AnimationsList::editSelected_setDurationFormat(MSA::DurationFormat durationFormat)
{
    return AnimationsListUndoHelper(this).editSelectedItemField(
        durationFormat, tr("Change Animation Duration Format"),
        [](MSA::Animation& a) -> MSA::DurationFormat& { return a.durationFormat; });
}

bool AnimationsList::editSelected_setOneShot(bool oneShot)
{
    QString text = oneShot ? tr("Set Animation One Shot") : tr("Clear Animation One Shot");

    return AnimationsListUndoHelper(this).editSelectedItemField(
        oneShot, text,
        [](MSA::Animation& a) -> bool& { return a.oneShot; });
}

bool AnimationsList::editSelected_setNextAnimation(const idstring& nextAnimation)
{
    return AnimationsListUndoHelper(this).editSelectedItemMultipleFields(
        std::make_tuple(false, nextAnimation),
        tr("Change Next Animation"),
        [](MSA::Animation& a) { return std::tie(a.oneShot, a.nextAnimation); });
}

using AnimationFramesUndoHelper = ListUndoHelper<AnimationFramesList>;

bool AnimationFramesList::editSelectedList_setData(AnimationFramesList::index_type index, const AnimationFramesList::DataT& value)
{
    return AnimationFramesUndoHelper(this).editItem(index, value);
}
