/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationframecommands.h"
#include "animationframesmodel.h"
#include "gui-qt/metasprite/abstractdocument.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::Animation;

// AddRemoveAnimationFrame
// =======================

AddRemoveAnimationFrame::AddRemoveAnimationFrame(AbstractDocument* document,
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
    _document->animationFramesModel()->insertAnimationFrame(_animation, _index, _animationFrame);
}

void AddRemoveAnimationFrame::removeAnimationFrame()
{
    _document->animationFramesModel()->removeAnimationFrame(_animation, _index);
}

// AddAnimationFrame
// =================

AddAnimationFrame::AddAnimationFrame(AbstractDocument* document,
                                     MSA::Animation* animation)
    : AddRemoveAnimationFrame(document, animation,
                              animation->frames.size(), MSA::AnimationFrame(),
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

CloneAnimationFrame::CloneAnimationFrame(AbstractDocument* document,
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

RemoveAnimationFrame::RemoveAnimationFrame(AbstractDocument* document,
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

// RaiseAnimationFrame
// ===================

RaiseAnimationFrame::RaiseAnimationFrame(AbstractDocument* document,
                                         MSA::Animation* animation, unsigned index)
    : QUndoCommand(QCoreApplication::tr("Raise Animation Frame"))
    , _document(document)
    , _animation(animation)
    , _index(index)
{
}

void RaiseAnimationFrame::undo()
{
    _document->animationFramesModel()->lowerAnimationFrame(_animation, _index - 1);
}

void RaiseAnimationFrame::redo()
{
    _document->animationFramesModel()->raiseAnimationFrame(_animation, _index);
}

// LowerAnimationFrame
// ===================

LowerAnimationFrame::LowerAnimationFrame(AbstractDocument* document,
                                         MSA::Animation* animation, unsigned index)
    : QUndoCommand(QCoreApplication::tr("Lower Animation Frame"))
    , _document(document)
    , _animation(animation)
    , _index(index)
{
}

void LowerAnimationFrame::undo()
{
    _document->animationFramesModel()->raiseAnimationFrame(_animation, _index + 1);
}

void LowerAnimationFrame::redo()
{
    _document->animationFramesModel()->lowerAnimationFrame(_animation, _index);
}

// ChangeAnimationFrame
// ====================

ChangeAnimationFrame::ChangeAnimationFrame(AbstractDocument* document,
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
    _document->animationFramesModel()->setAnimationFrame(_animation, _index, _oldValue);
}

void ChangeAnimationFrame::redo()
{
    _document->animationFramesModel()->setAnimationFrame(_animation, _index, _newValue);
}
