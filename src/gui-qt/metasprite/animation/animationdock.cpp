/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationdock.h"
#include "accessors.h"
#include "managers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/abstractmsresourceitem.h"
#include "gui-qt/metasprite/animation/animationdock.ui.h"

#include <QCompleter>
#include <QMenu>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationDock::AnimationDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::AnimationDock)
    , _resourceItem(nullptr)
    , _animationManager(new AnimationManager(this))
    , _animationFramesManager(new AnimationFramesManager(this))
{
    _ui->setupUi(this);

    _ui->animationList->namedListActions()->populate(_ui->animationListButtons);

    _ui->animationProperties->setPropertyManager(_animationManager);

    _ui->animationFrames->setPropertyManager(_animationFramesManager);
    _ui->animationFrames->viewActions()->populate(_ui->animationFramesButtons);

    setEnabled(false);
}

AnimationDock::~AnimationDock() = default;

Accessor::NamedListActions* AnimationDock::animationListActions()
{
    return _ui->animationList->namedListActions();
}

Accessor::ListActions* AnimationDock::animationFrameActions()
{
    return _ui->animationFrames->viewActions();
}

Accessor::NamedListModel* AnimationDock::animationListModel()
{
    return _ui->animationList->namedListModel();
}

void AnimationDock::setResourceItem(AbstractMsResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    if (_resourceItem != nullptr) {
        _resourceItem->disconnect(this);
        _resourceItem->animationsList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    setEnabled(_resourceItem != nullptr);

    AnimationsList* animationsList = _resourceItem ? _resourceItem->animationsList() : nullptr;

    _animationManager->setResourceItem(resourceItem);
    _animationFramesManager->setResourceItem(resourceItem);
    _ui->animationList->setAccessor(animationsList);

    if (_resourceItem) {
        _ui->animationFrames->setColumnWidth(0, _ui->animationFrames->width() / 3);
        _ui->animationFrames->setColumnWidth(1, 45);
        _ui->animationFrames->setColumnWidth(2, 30);
        _ui->animationFrames->setColumnWidth(3, 0);
    }
}
