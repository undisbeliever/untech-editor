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

class AddRemoveAnimationFrame : public QUndoCommand {
protected:
    AddRemoveAnimationFrame(AbstractDocument* document,
                            MSA::Animation* animation, unsigned index,
                            const MSA::AnimationFrame&,
                            const QString& text);
    ~AddRemoveAnimationFrame() = default;

protected:
    void addAnimationFrame();
    void removeAnimationFrame();

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
    const unsigned _index;
    const MSA::AnimationFrame _animationFrame;
};

class AddAnimationFrame : public AddRemoveAnimationFrame {
public:
    AddAnimationFrame(AbstractDocument* document,
                      MSA::Animation* animation);
    ~AddAnimationFrame() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class CloneAnimationFrame : public AddRemoveAnimationFrame {
public:
    CloneAnimationFrame(AbstractDocument* document,
                        MSA::Animation* animation, unsigned index);
    ~CloneAnimationFrame() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RemoveAnimationFrame : public AddRemoveAnimationFrame {
public:
    RemoveAnimationFrame(AbstractDocument* document,
                         MSA::Animation* animation, unsigned index);
    ~RemoveAnimationFrame() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RaiseAnimationFrame : public QUndoCommand {
public:
    RaiseAnimationFrame(AbstractDocument* document,
                        MSA::Animation* animation, unsigned index);
    ~RaiseAnimationFrame() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
    const unsigned _index;
};

class LowerAnimationFrame : public QUndoCommand {
public:
    LowerAnimationFrame(AbstractDocument* document,
                        MSA::Animation* animation, unsigned index);
    ~LowerAnimationFrame() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
    const unsigned _index;
};

class ChangeAnimationFrame : public QUndoCommand {
public:
    ChangeAnimationFrame(AbstractDocument* document,
                         MSA::Animation* animation, unsigned index,
                         const MSA::AnimationFrame& newValue);
    ~ChangeAnimationFrame() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    AbstractDocument* _document;
    MSA::Animation* _animation;
    unsigned _index;
    const MSA::AnimationFrame _oldValue;
    const MSA::AnimationFrame _newValue;
};
}
}
}
}
