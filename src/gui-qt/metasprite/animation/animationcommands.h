/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/animation/animation.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractDocument;

namespace Animation {

namespace MSA = UnTech::MetaSprite::Animation;

class AddRemoveAnimation : public QUndoCommand {
protected:
    AddRemoveAnimation(AbstractDocument* document,
                       const idstring& animationId, std::unique_ptr<MSA::Animation> frame);
    ~AddRemoveAnimation() = default;

protected:
    void addAnimation();
    void removeAnimation();

private:
    AbstractDocument* _document;
    const idstring _animationId;
    std::unique_ptr<MSA::Animation> _animation;
};

class AddAnimation : public AddRemoveAnimation {
public:
    AddAnimation(AbstractDocument* document, const idstring& newId);
    ~AddAnimation() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class CloneAnimation : public AddRemoveAnimation {
public:
    CloneAnimation(AbstractDocument* document,
                   const idstring& existingId, const idstring& newId);
    ~CloneAnimation() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RemoveAnimation : public AddRemoveAnimation {
public:
    RemoveAnimation(AbstractDocument* document, const idstring& animationId);
    ~RemoveAnimation() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RenameAnimation : public QUndoCommand {
public:
    RenameAnimation(AbstractDocument* document,
                    const idstring& oldId, const idstring& newId);
    ~RenameAnimation() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    AbstractDocument* _document;
    const idstring _oldId;
    const idstring _newId;
};

class ChangeAnimationDurationFormat : public QUndoCommand {
public:
    ChangeAnimationDurationFormat(AbstractDocument* document, MSA::Animation* animation,
                                  const MSA::DurationFormat& format);
    ~ChangeAnimationDurationFormat() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
    const MSA::DurationFormat _oldDurationFormat;
    const MSA::DurationFormat _newDurationFormat;
};

class ChangeAnimationOneShot : public QUndoCommand {
public:
    ChangeAnimationOneShot(AbstractDocument* document, MSA::Animation* animation,
                           bool oneShot);
    ~ChangeAnimationOneShot() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
    const idstring _oldNextAnimation;
    const idstring _newNextAnimation;
    const bool _oldOneShot;
    const bool _newOneShot;
};

class ChangeAnimationNextAnimation : public QUndoCommand {
public:
    ChangeAnimationNextAnimation(AbstractDocument* document, MSA::Animation* animation,
                                 const idstring& nextAnimation);
    ~ChangeAnimationNextAnimation() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
    const idstring _oldNextAnimation;
    const idstring _newNextAnimation;
};
}
}
}
}
