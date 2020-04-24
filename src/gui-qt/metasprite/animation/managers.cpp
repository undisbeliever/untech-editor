/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "accessors.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/abstractmsresourceitem.h"

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::Animation;

const QStringList AnimationFramesManager::FLIP_STRINGS({ QString(),
                                                         QString::fromUtf8("hFlip"),
                                                         QString::fromUtf8("vFlip"),
                                                         QString::fromUtf8("hvFlip") });

AnimationManager::AnimationManager(QObject* parent)
    : PropertyListManager(parent)
    , _animationsList(nullptr)
{
    using Type = UnTech::GuiQt::PropertyType;

    const auto& dfMap = MSA::DurationFormat::enumMap;

    addProperty(tr("Name"), NAME, Type::IDSTRING);
    addProperty(tr("Duration Format"), DURATION_FORMAT, Type::COMBO, enumComboNames(dfMap), enumComboDataList(dfMap));
    addProperty(tr("One Shot"), ONE_SHOT, Type::BOOLEAN);
    addProperty(tr("Next Animation"), NEXT_ANIMATION, Type::STRING_COMBO);
}

void AnimationManager::setResourceItem(AbstractMsResourceItem* entityHitboxList)
{
    auto* animationsList = entityHitboxList ? entityHitboxList->animationsList() : nullptr;

    if (_animationsList) {
        _animationsList->disconnect(this);
    }
    _animationsList = animationsList;

    if (_animationsList) {
        connect(_animationsList, &AnimationsList::selectedIndexChanged,
                this, &AnimationManager::onSelectedAnimationChanged);
        connect(_animationsList, &AnimationsList::dataChanged,
                this, &AnimationManager::onFrameDataChanged);
        connect(_animationsList, &AnimationsList::listAboutToChange,
                this, &AnimationManager::listAboutToChange);
    }

    onSelectedAnimationChanged();
}

void AnimationManager::onSelectedAnimationChanged()
{
    setEnabled(_animationsList && _animationsList->isSelectedIndexValid());
    emit dataChanged();
}

void AnimationManager::onFrameDataChanged(size_t animationIndex)
{
    Q_ASSERT(_animationsList);
    if (animationIndex == _animationsList->selectedIndex()) {
        emit dataChanged();
    }
}

QVariant AnimationManager::data(int id) const
{
    if (_animationsList == nullptr) {
        return QVariant();
    }

    const MSA::Animation* animation = _animationsList->selectedItem();
    if (animation == nullptr) {
        return QVariant();
    }

    switch (static_cast<PropertyId>(id)) {
    case NAME:
        return QString::fromStdString(animation->name);

    case DURATION_FORMAT:
        return int(animation->durationFormat.value());

    case ONE_SHOT:
        return animation->oneShot;

    case NEXT_ANIMATION:
        if (!animation->oneShot) {
            return QString::fromStdString(animation->nextAnimation);
        }
        else {
            return QVariant();
        }
    }

    return QVariant();
}

void AnimationManager::updateParameters(int id, QVariant& param1, QVariant&) const
{
    if (_animationsList == nullptr) {
        return;
    }

    switch (static_cast<PropertyId>(id)) {
    case NAME:
    case DURATION_FORMAT:
    case ONE_SHOT:
        break;

    case NEXT_ANIMATION:
        if (const auto* animations = _animationsList->list()) {
            param1 = convertNameListWithBlank(*animations);
        }
        break;
    }
}

bool AnimationManager::setData(int id, const QVariant& value)
{
    if (_animationsList == nullptr) {
        return false;
    }

    switch (static_cast<PropertyId>(id)) {
    case NAME:
        return _animationsList->editSelected_setName(value.toString().toStdString());

    case DURATION_FORMAT:
        return _animationsList->editSelected_setDurationFormat(static_cast<MSA::DurationFormat::Enum>(value.toInt()));

    case ONE_SHOT:
        return _animationsList->editSelected_setOneShot(value.toBool());

    case NEXT_ANIMATION:
        return _animationsList->editSelected_setNextAnimation(value.toString().toStdString());
    }

    return false;
}

AnimationFramesManager::AnimationFramesManager(QObject* parent)
    : Accessor::ListAccessorTableManager(parent)
    , _resourceItem(nullptr)
{
    using Type = PropertyType;

    addProperty(tr("Frame"), PropertyId::FRAME, Type::IDSTRING);
    addProperty(tr("Flip"), PropertyId::FLIP, Type::COMBO, FLIP_STRINGS, QVariantList{ 0, 1, 2, 3 });
    addProperty(tr("Dur"), PropertyId::DURATION, Type::UNSIGNED);
    addProperty(tr("Duration"), PropertyId::DURATION_STRING, Type::STRING);
}

void AnimationFramesManager::setResourceItem(AbstractMsResourceItem* resourceItem)
{
    _resourceItem = resourceItem;
    setAccessor(resourceItem ? resourceItem->animationFramesList() : nullptr);
}

inline const MSA::Animation* AnimationFramesManager::selectedAnimation() const
{
    return _resourceItem ? _resourceItem->animationsList()->selectedItem() : nullptr;
}

void AnimationFramesManager::updateParameters(int index, int id, QVariant& param1, QVariant& param2) const
{
    Q_UNUSED(param2);

    const MSA::Animation* animation = selectedAnimation();
    if (animation == nullptr
        || index < 0
        || (unsigned)index >= animation->frames.size()) {

        return;
    }

    switch ((PropertyId)id) {
    case PropertyId::FRAME:
        param1 = _resourceItem->frameNames();
        break;

    case PropertyId::FLIP:
    case PropertyId::DURATION:
    case PropertyId::DURATION_STRING:
        break;
    };
}

QVariant AnimationFramesManager::data(int index, int id) const
{
    const MSA::Animation* animation = selectedAnimation();
    if (animation == nullptr
        || index < 0
        || (unsigned)index >= animation->frames.size()) {

        return QVariant();
    }

    const MSA::AnimationFrame& aFrame = animation->frames.at(index);
    unsigned flipIndex = (aFrame.frame.vFlip << 1) | aFrame.frame.hFlip;

    switch ((PropertyId)id) {
    case PropertyId::FRAME:
        return QString::fromStdString(aFrame.frame.name);

    case PropertyId::FLIP:
        return flipIndex;

    case PropertyId::DURATION:
        return aFrame.duration;

    case PropertyId::DURATION_STRING: {
        return QString::fromStdString(animation->durationFormat.durationToString(aFrame.duration));
    }
    };

    return QVariant();
}

bool AnimationFramesManager::setData(int index, int id, const QVariant& value)
{
    const MSA::Animation* animation = selectedAnimation();
    if (animation == nullptr
        || index < 0
        || (unsigned)index >= animation->frames.size()) {

        return false;
    }

    MSA::AnimationFrame aFrame = animation->frames.at(index);

    switch ((PropertyId)id) {
    case PropertyId::FRAME:
        aFrame.frame.name = value.toString().toStdString();
        break;

    case PropertyId::FLIP:
        aFrame.frame.hFlip = value.toUInt() & 1;
        aFrame.frame.vFlip = value.toUInt() & 2;
        break;

    case PropertyId::DURATION:
        aFrame.duration = value.toUInt();
        break;

    case PropertyId::DURATION_STRING:
        return false;
    };

    return _resourceItem->animationFramesList()->editSelectedList_setData(index, aFrame);
}
