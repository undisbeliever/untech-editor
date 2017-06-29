/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/animation/animation.h"
#include <QCompleter>
#include <QDockWidget>
#include <QItemSelection>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractDocument;

namespace Animation {
namespace Ui {
class AnimationDock;
}
class AnimationActions;

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationDock : public QDockWidget {
    Q_OBJECT

public:
    explicit AnimationDock(QWidget* parent = nullptr);
    ~AnimationDock();

    AnimationActions* actions() const { return _actions.get(); }

    void setDocument(AbstractDocument* document);

    void clearGui();

private slots:
    void onSelectedAnimationChanged();
    void onSelectedAnimationFrameChanged();

    void onAnimationDataChanged(const void* animation);

    void updateGui();

    void onDurationFormatEdited();
    void onOneShotEdited();
    void onNextAnimationEdited();

    void onAnimationListSelectionChanged();
    void onAnimationFrameSelectionChanged();

    void onAnimationListContextMenu(const QPoint& pos);

private:
    std::unique_ptr<Ui::AnimationDock> _ui;
    std::unique_ptr<AnimationActions> _actions;
    AbstractDocument* _document;

    QCompleter* _nextAnimationCompleter;
};
}
}
}
}
