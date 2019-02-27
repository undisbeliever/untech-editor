/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "managers.h"
#include "animationaccessors.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/abstractmsdocument.h"

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
    addProperty(tr("Next Animation"), NEXT_ANIMATION, Type::COMBO);
}

void AnimationManager::setDocument(AbstractMsDocument* document)
{
    auto* animationsList = document ? document->animationsList() : nullptr;

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

    const MSA::Animation* animation = _animationsList->selectedAnimation();
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
    : PropertyTableManager(parent)
    , _document(nullptr)
{
    using Type = PropertyType;

    setItemsMovable(true);

    addProperty(tr("Frame"), PropertyId::FRAME, Type::IDSTRING);
    addProperty(tr("Flip"), PropertyId::FLIP, Type::COMBO, FLIP_STRINGS, QVariantList{ 0, 1, 2, 3 });
    addProperty(tr("Dur"), PropertyId::DURATION, Type::UNSIGNED);
    addProperty(tr("Duration"), PropertyId::DURATION_STRING, Type::STRING);
}

void AnimationFramesManager::setDocument(AbstractMsDocument* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->animationsList()->disconnect(this);
        _document->animationFramesList()->disconnect(this);
    }
    _document = document;

    emit dataReset();

    if (_document) {
        onSelectedAnimationChanged();

        connect(_document->animationsList(), &AnimationsList::dataChanged,
                this, &AnimationFramesManager::onAnimationDataChanged);

        connect(_document->animationFramesList(), &AnimationFramesList::selectedListChanged,
                this, &AnimationFramesManager::onSelectedAnimationChanged);

        connect(_document->animationFramesList(), &AnimationFramesList::dataChanged,
                this, &AnimationFramesManager::onAnimationFrameChanged);
        connect(_document->animationFramesList(), &AnimationFramesList::itemAdded,
                this, &AnimationFramesManager::onAnimationFrameAdded);
        connect(_document->animationFramesList(), &AnimationFramesList::itemAboutToBeRemoved,
                this, &AnimationFramesManager::onAnimationFrameAboutToBeRemoved);
        connect(_document->animationFramesList(), &AnimationFramesList::itemMoved,
                this, &AnimationFramesManager::onAnimationFrameMoved);

        connect(_document->animationFramesList(), &AnimationFramesList::listAboutToChange,
                this, &AnimationFramesManager::listAboutToChange);
    }
}

const MSA::Animation* AnimationFramesManager::selectedAnimation() const
{
    if (_document == nullptr) {
        return nullptr;
    }
    return _document->animationsList()->selectedAnimation();
}

void AnimationFramesManager::updateParameters(int index, int id, QVariant& param1, QVariant& param2) const
{
    Q_UNUSED(param2);

    const MSA::Animation* animation = _document->animationsList()->selectedAnimation();
    if (animation == nullptr
        || index < 0
        || (unsigned)index >= animation->frames.size()) {

        return;
    }

    switch ((PropertyId)id) {
    case PropertyId::FRAME:
        param1 = _document->frameNames();
        break;

    case PropertyId::FLIP:
    case PropertyId::DURATION:
    case PropertyId::DURATION_STRING:
        break;
    };
}

void AnimationFramesManager::onSelectedAnimationChanged()
{
    emit dataReset();
}

void AnimationFramesManager::onAnimationDataChanged(size_t animationIndex)
{
    Q_ASSERT(_document);
    if (animationIndex == _document->animationsList()->selectedIndex()) {
        emit dataChanged();
    }
}

void AnimationFramesManager::onAnimationFrameChanged(size_t animationIndex, unsigned index)
{
    Q_ASSERT(_document);
    if (animationIndex == _document->animationsList()->selectedIndex()) {
        emit itemChanged(index);
    }
}

void AnimationFramesManager::onAnimationFrameAdded(size_t animationIndex, unsigned index)
{
    Q_ASSERT(_document);
    if (animationIndex == _document->animationsList()->selectedIndex()) {
        emit itemAdded(index);
    }
}

void AnimationFramesManager::onAnimationFrameAboutToBeRemoved(size_t animationIndex, unsigned index)
{
    Q_ASSERT(_document);
    if (animationIndex == _document->animationsList()->selectedIndex()) {
        emit itemRemoved(index);
    }
}

void AnimationFramesManager::onAnimationFrameMoved(size_t animationIndex, unsigned oldPos, unsigned newPos)
{
    Q_ASSERT(_document);
    if (animationIndex == _document->animationsList()->selectedIndex()) {
        emit itemMoved(oldPos, newPos);
    }
}

int AnimationFramesManager::rowCount() const
{
    const MSA::Animation* animation = selectedAnimation();
    if (animation) {
        return animation->frames.size();
    }
    else {
        return 0;
    }
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

    return _document->animationFramesList()->editSelectedList_setData(index, aFrame);
}

bool AnimationFramesManager::canInsertItem()
{
    const MSA::Animation* animation = selectedAnimation();

    return animation != nullptr
           && animation->frames.size() < UnTech::MetaSprite::MAX_ANIMATION_FRAMES;
}

bool AnimationFramesManager::canCloneItem(int index)
{
    const MSA::Animation* animation = selectedAnimation();

    return animation != nullptr
           && animation->frames.size() < UnTech::MetaSprite::MAX_ANIMATION_FRAMES
           && index >= 0 && (unsigned)index < animation->frames.size();
}

bool AnimationFramesManager::insertItem(int index)
{
    if (_document == nullptr) {
        return false;
    }
    return _document->animationFramesList()->editSelectedList_addItem(index);
}

bool AnimationFramesManager::cloneItem(int index)
{
    if (_document == nullptr) {
        return false;
    }
    return _document->animationFramesList()->editSelectedList_cloneItem(index);
}

bool AnimationFramesManager::removeItem(int index)
{
    if (_document == nullptr) {
        return false;
    }
    return _document->animationFramesList()->editSelectedList_removeItem(index);
}

bool AnimationFramesManager::moveItem(int from, int to)
{
    if (_document == nullptr) {
        return false;
    }
    return _document->animationFramesList()->editSelectedList_moveItem(from, to);
}
