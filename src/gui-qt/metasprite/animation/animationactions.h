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
class AbstractDocument;

namespace Animation {

class AnimationActions : public QObject {
    Q_OBJECT

public:
    AnimationActions(QWidget* parent);
    ~AnimationActions() = default;

    void setDocument(AbstractDocument*);

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
    QWidget* _widget;
    AbstractDocument* _document;

    QAction* _addAnimation;
    QAction* _cloneAnimation;
    QAction* _renameAnimation;
    QAction* _removeAnimation;
};
}
}
}
}
