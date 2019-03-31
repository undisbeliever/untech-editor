/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "models/metasprite/animation/animation.h"
#include <QDockWidget>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractMsDocument;

namespace Animation {
namespace Ui {
class AnimationDock;
}
class AnimationActions;
class AnimationListModel;
class AnimationManager;
class AnimationFramesManager;

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationDock : public QDockWidget {
    Q_OBJECT

public:
    explicit AnimationDock(QWidget* parent = nullptr);
    ~AnimationDock();

    Accessor::NamedListActions* animationListActions();
    Accessor::ListActions* animationFrameActions();
    Accessor::NamedListModel* animationListModel();

    void setDocument(AbstractMsDocument* document);

private:
    std::unique_ptr<Ui::AnimationDock> const _ui;

    AbstractMsDocument* _document;

    AnimationManager* const _animationManager;
    AnimationFramesManager* const _animationFramesManager;
};
}
}
}
}
