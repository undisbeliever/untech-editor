/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationaccessors.h"
#include "gui-qt/accessor/selectedindexhelper.h"
#include "gui-qt/common/helpers.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationsList::AnimationsList(AbstractMsDocument* document)
    : QObject(document)
    , _document(document)
    , _selectedIndex()
{
    SelectedIndexHelper::buildAndConnectSlots_NamedList(this);
}

QStringList AnimationsList::animationNames() const
{
    if (const auto* aList = _document->animations()) {
        return convertNameList(*aList);
    }
    else {
        return QStringList();
    }
}

void AnimationsList::setSelectedId(const UnTech::idstring& id)
{
    if (auto* list = _document->animations()) {
        setSelectedIndex(list->indexOf(id));
    }
    else {
        unselectItem();
    }
}

void AnimationsList::setSelectedIndex(const AnimationsList::index_type& index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

bool AnimationsList::isSelectedIndexValid() const
{
    auto* animations = _document->animations();

    return animations
           && _selectedIndex < animations->size();
}

const UnTech::MetaSprite::Animation::Animation* AnimationsList::selectedAnimation() const
{
    auto* animations = _document->animations();
    if (_selectedIndex >= animations->size()) {
        return nullptr;
    }
    return &animations->at(_selectedIndex);
}

UnTech::MetaSprite::Animation::Animation* AnimationsList::selectedItemEditable()
{
    auto* animations = _document->animations();
    if (_selectedIndex >= animations->size()) {
        return nullptr;
    }
    return &animations->at(_selectedIndex);
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
