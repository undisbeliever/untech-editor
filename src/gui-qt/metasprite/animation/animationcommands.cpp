/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationcommands.h"
#include "animationlistmodel.h"
#include "gui-qt/metasprite/abstractmsdocument.h"

#include <QCoreApplication>

using namespace UnTech;
using namespace UnTech::GuiQt::MetaSprite::Animation;

// AddRemoveAnimation
// ==================

AddRemoveAnimation::AddRemoveAnimation(AbstractMsDocument* document,
                                       const idstring& animationId, std::unique_ptr<MSA::Animation> frame)
    : QUndoCommand()
    , _document(document)
    , _animationId(animationId)
    , _animation(std::move(frame))
{
    Q_ASSERT(_animationId.isValid());
}

void AddRemoveAnimation::addAnimation()
{
    Q_ASSERT(_animation != nullptr);
    Q_ASSERT(_document->animations()->contains(_animationId) == false);

    _document->animationListModel()->insertAnimation(_animationId, std::move(_animation));
}

void AddRemoveAnimation::removeAnimation()
{
    Q_ASSERT(_animation == nullptr);
    Q_ASSERT(_document->animations()->contains(_animationId));

    _animation = _document->animationListModel()->removeAnimation(_animationId);
}

// AddAnimation
// ============

AddAnimation::AddAnimation(AbstractMsDocument* document, const idstring& newId)
    : AddRemoveAnimation(document, newId,
                         std::make_unique<MSA::Animation>())
{
    setText(QCoreApplication::tr("Add Animation"));
}

void AddAnimation::undo()
{
    removeAnimation();
}
void AddAnimation::redo()
{
    addAnimation();
}

// CloneAnimation
// ==============

CloneAnimation::CloneAnimation(AbstractMsDocument* document,
                               const idstring& id, const idstring& newId)
    : AddRemoveAnimation(document, newId,
                         std::make_unique<MSA::Animation>(document->animations()->at(id)))
{
    setText(QCoreApplication::tr("Clone Animation"));
}

void CloneAnimation::undo()
{
    removeAnimation();
}
void CloneAnimation::redo()
{
    addAnimation();
}

// RemoveAnimation
// ===============

RemoveAnimation::RemoveAnimation(AbstractMsDocument* document, const idstring& animationId)
    : AddRemoveAnimation(document, animationId, nullptr)
{
    setText(QCoreApplication::tr("Remove Animation"));
}

void RemoveAnimation::undo()
{
    addAnimation();
}
void RemoveAnimation::redo()
{
    removeAnimation();
}

// RenameAnimation
// ===============

RenameAnimation::RenameAnimation(AbstractMsDocument* document,
                                 const idstring& oldId, const idstring& newId)
    : QUndoCommand(QCoreApplication::tr("Rename Animation"))
    , _document(document)
    , _oldId(oldId)
    , _newId(newId)
{
    Q_ASSERT(_oldId != _newId);
}

void RenameAnimation::undo()
{
    _document->animationListModel()->renameAnimation(_newId, _oldId);
}

void RenameAnimation::redo()
{
    _document->animationListModel()->renameAnimation(_oldId, _newId);
}

// ChangeAnimationDurationFormat
// =============================

ChangeAnimationDurationFormat::ChangeAnimationDurationFormat(
    AbstractMsDocument* document, MSA::Animation* animation, const MSA::DurationFormat& format)
    : QUndoCommand(QCoreApplication::tr("Change Animation Duration Format"))
    , _document(document)
    , _animation(animation)
    , _oldDurationFormat(animation->durationFormat)
    , _newDurationFormat(format)
{
    Q_ASSERT(_oldDurationFormat != _newDurationFormat);
}

void ChangeAnimationDurationFormat::undo()
{
    _animation->durationFormat = _oldDurationFormat;
    emit _document->animationDataChanged(_animation);
}

void ChangeAnimationDurationFormat::redo()
{
    _animation->durationFormat = _newDurationFormat;
    emit _document->animationDataChanged(_animation);
}

// ChangeAnimationNextAnimation
// ============================

ChangeAnimationNextAnimation::ChangeAnimationNextAnimation(
    AbstractMsDocument* document, MSA::Animation* animation, const idstring& nextAnimation)
    : QUndoCommand(QCoreApplication::tr("Change Next Animation"))
    , _document(document)
    , _animation(animation)
    , _oldNextAnimation(animation->nextAnimation)
    , _newNextAnimation(nextAnimation)
{
    Q_ASSERT(_oldNextAnimation != _newNextAnimation);
}

void ChangeAnimationNextAnimation::undo()
{
    _animation->nextAnimation = _oldNextAnimation;
    emit _document->animationDataChanged(_animation);
}

void ChangeAnimationNextAnimation::redo()
{
    _animation->nextAnimation = _newNextAnimation;
    emit _document->animationDataChanged(_animation);
}

// ChangeAnimationOneShot
// ======================

ChangeAnimationOneShot::ChangeAnimationOneShot(
    AbstractMsDocument* document, MSA::Animation* animation, bool oneShot)
    : QUndoCommand()
    , _document(document)
    , _animation(animation)
    , _oldNextAnimation(animation->nextAnimation)
    , _newNextAnimation(oneShot ? idstring() : _oldNextAnimation)
    , _oldOneShot(animation->oneShot)
    , _newOneShot(oneShot)
{
    Q_ASSERT(_oldOneShot != _newOneShot);

    if (oneShot) {
        setText(QCoreApplication::tr("Set Animation One Shot"));
    }
    else {
        setText(QCoreApplication::tr("Clear Animation One Shot"));
    }
}

void ChangeAnimationOneShot::undo()
{
    _animation->oneShot = _oldOneShot;
    _animation->nextAnimation = _oldNextAnimation;
    emit _document->animationDataChanged(_animation);
}

void ChangeAnimationOneShot::redo()
{
    _animation->oneShot = _newOneShot;
    _animation->nextAnimation = _newNextAnimation;
    emit _document->animationDataChanged(_animation);
}
