/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationaccessors.h"
#include "gui-qt/accessor/listundohelper.h"
#include "gui-qt/accessor/namedlistundohelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::Animation;

using AnimationsListUndoHelper = NamedListAndSelectionUndoHelper<AnimationsList>;

bool AnimationsList::editSelected_setDurationFormat(MSA::DurationFormat durationFormat)
{
    AnimationsListUndoHelper helper(this);
    return helper.editSelectedItemField(durationFormat, tr("Change Animation Duration Format"),
                                        [](MSA::Animation& a) -> MSA::DurationFormat& { return a.durationFormat; });
}

bool AnimationsList::editSelected_setOneShot(bool oneShot)
{
    QString text = oneShot ? tr("Set Animation One Shot") : tr("Clear Animation One Shot");

    AnimationsListUndoHelper helper(this);
    return helper.editSelectedItemField(oneShot, text,
                                        [](MSA::Animation& a) -> bool& { return a.oneShot; });
}

bool AnimationsList::editSelected_setNextAnimation(const idstring& nextAnimation)
{
    AnimationsListUndoHelper helper(this);
    return helper.editSelectedItemField(nextAnimation, tr("Change Next Animation"),
                                        [](MSA::Animation& a) -> idstring& { return a.nextAnimation; });
}

using AnimationFramesUndoHelper = ListUndoHelper<AnimationFramesList>;

bool AnimationFramesList::editSelectedList_setData(AnimationFramesList::index_type index, const AnimationFramesList::DataT& value)
{
    return AnimationFramesUndoHelper(this).editItemInSelectedList(index, value);
}

bool AnimationFramesList::editSelectedList_addItem(index_type index)
{
    return AnimationFramesUndoHelper(this).addItemToSelectedList(index);
}

bool AnimationFramesList::editSelectedList_cloneItem(index_type index)
{
    return AnimationFramesUndoHelper(this).cloneItemInSelectedList(index);
}

bool AnimationFramesList::editSelectedList_removeItem(index_type index)
{
    return AnimationFramesUndoHelper(this).removeItemFromSelectedList(index);
}

bool AnimationFramesList::editSelectedList_moveItem(index_type from, index_type to)
{
    return AnimationFramesUndoHelper(this).moveItemInSelectedList(from, to);
}
