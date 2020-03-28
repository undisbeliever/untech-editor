/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
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
class AbstractMsResourceItem;

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

    void setResourceItem(AbstractMsResourceItem* resourceItem);

private:
    std::unique_ptr<Ui::AnimationDock> const _ui;

    AbstractMsResourceItem* _resourceItem;

    AnimationManager* const _animationManager;
    AnimationFramesManager* const _animationFramesManager;
};
}
}
}
}
