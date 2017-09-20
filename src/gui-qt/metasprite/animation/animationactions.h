/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QObject>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsDocument;

namespace Animation {

class AnimationActions : public QObject {
    Q_OBJECT

public:
    AnimationActions(QWidget* parent);
    ~AnimationActions() = default;

    void setDocument(AbstractMsDocument*);

    QAction* addAnimation() const { return _addAnimation; }
    QAction* cloneAnimation() const { return _cloneAnimation; }
    QAction* renameAnimation() const { return _renameAnimation; }
    QAction* removeAnimation() const { return _removeAnimation; }

    QAction* addAnimationFrame() const { return _addAnimationFrame; }
    QAction* raiseAnimationFrame() const { return _raiseAnimationFrame; }
    QAction* lowerAnimationFrame() const { return _lowerAnimationFrame; }
    QAction* cloneAnimationFrame() const { return _cloneAnimationFrame; }
    QAction* removeAnimationFrame() const { return _removeAnimationFrame; }

public slots:
    void updateActions();

    void onAddAnimation();
    void onCloneAnimation();
    void onRenameAnimation();
    void onRemoveAnimation();

    void onAddAnimationFrame();
    void onRaiseAnimationFrame();
    void onLowerAnimationFrame();
    void onCloneAnimationFrame();
    void onRemoveAnimationFrame();

private:
    QWidget* _widget;
    AbstractMsDocument* _document;

    QAction* _addAnimation;
    QAction* _cloneAnimation;
    QAction* _renameAnimation;
    QAction* _removeAnimation;

    QAction* _addAnimationFrame;
    QAction* _raiseAnimationFrame;
    QAction* _lowerAnimationFrame;
    QAction* _cloneAnimationFrame;
    QAction* _removeAnimationFrame;
};
}
}
}
}
