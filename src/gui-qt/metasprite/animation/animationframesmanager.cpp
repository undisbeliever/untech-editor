/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationframesmanager.h"
#include "animationframecommands.h"
#include "gui-qt/common/abstractidmaplistmodel.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/abstractselection.h"

using namespace UnTech::GuiQt::MetaSprite::Animation;

const QStringList AnimationFramesManager::FLIP_STRINGS({ QString(),
                                                         QString::fromUtf8("hFlip"),
                                                         QString::fromUtf8("vFlip"),
                                                         QString::fromUtf8("hvFlip") });

AnimationFramesManager::AnimationFramesManager(QObject* parent)
    : PropertyTableManager(parent)
    , _document(nullptr)
    , _animation(nullptr)
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
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    onSelectedAnimationChanged();
    connect(_document->selection(), &AbstractSelection::selectedAnimationChanged,
            this, &AnimationFramesManager::onSelectedAnimationChanged);

    connect(_document, &AbstractMsDocument::animationDataChanged,
            this, &AnimationFramesManager::onAnimationDataChanged);

    connect(_document, &AbstractMsDocument::animationFrameChanged,
            this, &AnimationFramesManager::onAnimationFrameChanged);
    connect(_document, &AbstractMsDocument::animationFrameAdded,
            this, &AnimationFramesManager::onAnimationFrameAdded);
    connect(_document, &AbstractMsDocument::animationFrameAboutToBeRemoved,
            this, &AnimationFramesManager::onAnimationFrameAboutToBeRemoved);
    connect(_document, &AbstractMsDocument::animationFrameMoved,
            this, &AnimationFramesManager::onAnimationFrameMoved);
}

void AnimationFramesManager::updateParameters(int index, int id, QVariant& param1, QVariant& param2) const
{
    Q_UNUSED(param2);

    if (_animation == nullptr
        || index < 0
        || (unsigned)index >= _animation->frames.size()) {

        return;
    }

    switch ((PropertyId)id) {
    case PropertyId::FRAME:
        param1 = _document->frameListModel()->displayList();
        break;

    case PropertyId::FLIP:
    case PropertyId::DURATION:
    case PropertyId::DURATION_STRING:
        break;
    };
}

void AnimationFramesManager::onSelectedAnimationChanged()
{
    MSA::Animation* animation = _document->selection()->selectedAnimation();

    if (_animation != animation) {
        _animation = animation;
        emit dataReset();
    }
}

void AnimationFramesManager::onAnimationDataChanged(const void* animation)
{
    if (animation == _animation) {
        emit dataChanged();
    }
}

void AnimationFramesManager::onAnimationFrameChanged(const void* animation, unsigned index)
{
    if (animation == _animation) {
        emit itemChanged(index);
    }
}

void AnimationFramesManager::onAnimationFrameAdded(const void* animation, unsigned index)
{
    if (animation == _animation) {
        emit itemAdded(index);
    }
}

void AnimationFramesManager::onAnimationFrameAboutToBeRemoved(const void* animation, unsigned index)
{
    if (animation == _animation) {
        emit itemRemoved(index);
    }
}

void AnimationFramesManager::onAnimationFrameMoved(const void* animation, unsigned oldPos, unsigned newPos)
{
    if (animation == _animation) {
        emit itemMoved(oldPos, newPos);
    }
}

int AnimationFramesManager::rowCount() const
{
    if (_animation) {
        return _animation->frames.size();
    }
    else {
        return 0;
    }
}

QVariant AnimationFramesManager::data(int index, int id) const
{
    if (_animation == nullptr
        || index < 0
        || (unsigned)index >= _animation->frames.size()) {

        return QVariant();
    }

    const MSA::AnimationFrame& aFrame = _animation->frames.at(index);
    unsigned flipIndex = (aFrame.frame.vFlip << 1) | aFrame.frame.hFlip;

    switch ((PropertyId)id) {
    case PropertyId::FRAME:
        return QString::fromStdString(aFrame.frame.name);

    case PropertyId::FLIP:
        return flipIndex;

    case PropertyId::DURATION:
        return aFrame.duration;

    case PropertyId::DURATION_STRING: {
        return QString::fromStdString(_animation->durationFormat.durationToString(aFrame.duration));
    }
    };

    return QVariant();
}

bool AnimationFramesManager::setData(int index, int id, const QVariant& value)
{
    if (_animation == nullptr
        || index < 0
        || (unsigned)index >= _animation->frames.size()) {

        return false;
    }

    const auto& oldFrame = _animation->frames.at(index);
    MSA::AnimationFrame aFrame = oldFrame;

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

    if (aFrame != oldFrame) {
        _document->undoStack()->push(
            new ChangeAnimationFrame(_document, _animation, index, aFrame));

        return true;
    }

    return false;
}

bool AnimationFramesManager::canInsertItem()
{
    return _animation != nullptr
           && _animation->frames.can_insert();
}

bool AnimationFramesManager::canCloneItem(int index)
{
    return _animation != nullptr
           && _animation->frames.can_insert()
           && index >= 0 && (unsigned)index < _animation->frames.size();
}

bool AnimationFramesManager::insertItem(int index)
{
    if (_animation == nullptr
        || _animation->frames.can_insert() == false
        || index < 0 || (unsigned)index > _animation->frames.size()) {

        return false;
    }

    _document->undoStack()->push(
        new AddAnimationFrame(_document, _animation, index));

    return true;
}

bool AnimationFramesManager::cloneItem(int index)
{
    if (_animation == nullptr
        || _animation->frames.can_insert() == false
        || index < 0 || (unsigned)index > _animation->frames.size()) {

        return false;
    }

    _document->undoStack()->push(
        new CloneAnimationFrame(_document, _animation, index));

    return true;
}

bool AnimationFramesManager::removeItem(int index)
{
    if (_animation == nullptr
        || index < 0 || (unsigned)index >= _animation->frames.size()) {

        return false;
    }

    _document->undoStack()->push(
        new RemoveAnimationFrame(_document, _animation, index));

    return true;
}

bool AnimationFramesManager::moveItem(int from, int to)
{
    if (_animation == nullptr
        || from == to
        || from < 0 || (unsigned)from >= _animation->frames.size()
        || to < 0 || (unsigned)to >= _animation->frames.size()) {

        return false;
    }

    _document->undoStack()->push(
        new MoveAnimationFrame(_document, _animation, from, to));

    return true;
}
