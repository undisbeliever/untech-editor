/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationaccessors.h"
#include "gui-qt/accessor/abstractaccessors.hpp"
#include "gui-qt/common/helpers.h"

using namespace UnTech;
using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::Animation;

template <>
const NamedList<MSA::Animation>* NamedListAccessor<MSA::Animation, AbstractMsDocument>::list() const
{
    return _resourceItem->animations();
}

template <>
NamedList<MSA::Animation>* NamedListAccessor<MSA::Animation, AbstractMsDocument>::getList()
{
    return _resourceItem->animations();
}

AnimationsList::AnimationsList(AbstractMsDocument* document)
    : NamedListAccessor(document, UnTech::MetaSprite::MAX_EXPORT_NAMES)
{
}

QString AnimationsList::typeName() const
{
    return tr("Animation");
}

AnimationFramesList::AnimationFramesList(AbstractMsDocument* document)
    : QObject(document)
    , _document(document)
    , _animationIndex(INT_MAX)
{
    connect(_document->animationsList(), &AnimationsList::selectedIndexChanged,
            this, [this] {
                _animationIndex = _document->animationsList()->selectedIndex();
                emit selectedListChanged();
            });
}
