/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationframecommands.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "models/common/vector-helpers.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::Animation;

// AddRemoveAnimationFrame
// =======================

AddRemoveAnimationFrame::AddRemoveAnimationFrame(AbstractMsDocument* document,
                                                 MSA::Animation* animation, unsigned index,
                                                 const MSA::AnimationFrame& animationFrame,
                                                 const QString& text)
    : QUndoCommand(text)
    , _document(document)
    , _animation(animation)
    , _index(index)
    , _animationFrame(animationFrame)
{
    Q_ASSERT(_animation != nullptr);
}

void AddRemoveAnimationFrame::addAnimationFrame()
{
    Q_ASSERT(_index <= _animation->frames.size());

    auto it = _animation->frames.begin() + _index;
    _animation->frames.insert(it, _animationFrame);

    emit _document->animationFrameAdded(_animation, _index);
    emit _document->animationFrameListChanged(_animation);
}

void AddRemoveAnimationFrame::removeAnimationFrame()
{
    Q_ASSERT(_index <= _animation->frames.size());

    emit _document->animationFrameAboutToBeRemoved(_animation, _index);

    auto it = _animation->frames.begin() + _index;
    _animation->frames.erase(it);

    emit _document->animationFrameListChanged(_animation);
}

// AddAnimationFrame
// =================

AddAnimationFrame::AddAnimationFrame(AbstractMsDocument* document,
                                     MSA::Animation* animation)
    : AddAnimationFrame(document, animation, animation->frames.size())
{
}

AddAnimationFrame::AddAnimationFrame(AbstractMsDocument* document,
                                     MSA::Animation* animation, int index)
    : AddRemoveAnimationFrame(document, animation,
                              index, MSA::AnimationFrame(),
                              QCoreApplication::tr("Add Animation Frame"))
{
}

void AddAnimationFrame::undo()
{
    removeAnimationFrame();
}
void AddAnimationFrame::redo()
{
    addAnimationFrame();
}

// CloneAnimationFrame
// ===================

CloneAnimationFrame::CloneAnimationFrame(AbstractMsDocument* document,
                                         MSA::Animation* animation, unsigned index)
    : AddRemoveAnimationFrame(document, animation,
                              animation->frames.size(), animation->frames.at(index),
                              QCoreApplication::tr("Clone Animation Frame"))
{
}

void CloneAnimationFrame::undo()
{
    removeAnimationFrame();
}
void CloneAnimationFrame::redo()
{
    addAnimationFrame();
}

// RemoveAnimationFrame
// ====================

RemoveAnimationFrame::RemoveAnimationFrame(AbstractMsDocument* document,
                                           MSA::Animation* animation, unsigned index)
    : AddRemoveAnimationFrame(document, animation,
                              index, animation->frames.at(index),
                              QCoreApplication::tr("Remove Animation Frame"))
{
}

void RemoveAnimationFrame::undo()
{
    addAnimationFrame();
}
void RemoveAnimationFrame::redo()
{
    removeAnimationFrame();
}

// MoveAnimationFrame
// ===================

MoveAnimationFrame::MoveAnimationFrame(AbstractMsDocument* document,
                                       MSA::Animation* animation,
                                       unsigned fromIndex, unsigned toIndex)
    : QUndoCommand(QCoreApplication::tr("Move Animation Frame"))
    , _document(document)
    , _animation(animation)
    , _fromIndex(fromIndex)
    , _toIndex(toIndex)
{
    Q_ASSERT(_fromIndex != _toIndex);
}

void MoveAnimationFrame::undo()
{
    Q_ASSERT(_fromIndex < _animation->frames.size());
    Q_ASSERT(_toIndex < _animation->frames.size());

    moveVectorItem(_toIndex, _fromIndex, _animation->frames);

    emit _document->animationFrameMoved(_animation, _toIndex, _fromIndex);
    emit _document->animationFrameListChanged(_animation);
}

void MoveAnimationFrame::redo()
{
    Q_ASSERT(_fromIndex < _animation->frames.size());
    Q_ASSERT(_toIndex < _animation->frames.size());

    moveVectorItem(_fromIndex, _toIndex, _animation->frames);

    emit _document->animationFrameMoved(_animation, _fromIndex, _toIndex);
    emit _document->animationFrameListChanged(_animation);
}

// ChangeAnimationFrame
// ====================

ChangeAnimationFrame::ChangeAnimationFrame(AbstractMsDocument* document,
                                           MSA::Animation* animation, unsigned index,
                                           const MSA::AnimationFrame& newValue)
    : QUndoCommand(QCoreApplication::tr("Change Animation Frame"))
    , _document(document)
    , _animation(animation)
    , _index(index)
    , _oldValue(animation->frames.at(index))
    , _newValue(newValue)
{
    Q_ASSERT(_oldValue != _newValue);
}

void ChangeAnimationFrame::undo()
{
    _animation->frames.at(_index) = _oldValue;

    emit _document->animationFrameChanged(_animation, _index);
}

void ChangeAnimationFrame::redo()
{
    _animation->frames.at(_index) = _newValue;

    emit _document->animationFrameChanged(_animation, _index);
}
