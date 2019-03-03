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
    return resourceItem()->animations();
}

template <>
NamedList<MSA::Animation>* NamedListAccessor<MSA::Animation, AbstractMsDocument>::getList()
{
    return resourceItem()->animations();
}

AnimationsList::AnimationsList(AbstractMsDocument* document)
    : NamedListAccessor(document, UnTech::MetaSprite::MAX_EXPORT_NAMES)
{
}

QString AnimationsList::typeName() const
{
    return tr("Animation");
}

template <>
const std::vector<MSA::AnimationFrame>* ChildVectorAccessor<MSA::AnimationFrame, AbstractMsDocument>::list(size_t pIndex) const
{
    const auto* animations = resourceItem()->animations();
    if (animations == nullptr) {
        return nullptr;
    }
    if (pIndex >= animations->size()) {
        return nullptr;
    }
    return &animations->at(pIndex).frames;
}

template <>
std::vector<MSA::AnimationFrame>* ChildVectorAccessor<MSA::AnimationFrame, AbstractMsDocument>::getList(size_t pIndex)
{
    auto* animations = resourceItem()->animations();
    if (animations == nullptr) {
        return nullptr;
    }
    if (pIndex >= animations->size()) {
        return nullptr;
    }
    return &animations->at(pIndex).frames;
}

AnimationFramesList::AnimationFramesList(AbstractMsDocument* document)
    : ChildVectorAccessor(document->animationsList(), document, UnTech::MetaSprite::MAX_ANIMATION_FRAMES)
{
}

QString AnimationFramesList::typeName() const
{
    return tr("Animation Frame");
}

using namespace UnTech::GuiQt;
template class Accessor::NamedListAccessor<MSA::Animation, AbstractMsDocument>;
template class Accessor::ChildVectorAccessor<MSA::AnimationFrame, AbstractMsDocument>;
