/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

public slots:
    void updateActions();

    void onAddAnimation();
    void onCloneAnimation();
    void onRenameAnimation();
    void onRemoveAnimation();

private:
    QWidget* const _widget;

    AbstractMsDocument* _document;

    QAction* const _addAnimation;
    QAction* const _cloneAnimation;
    QAction* const _renameAnimation;
    QAction* const _removeAnimation;
};
}
}
}
}
