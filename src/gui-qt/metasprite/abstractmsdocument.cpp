/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractmsdocument.h"
#include "animation/animationframesmanager.h"
#include "animation/animationlistmodel.h"

using namespace UnTech::GuiQt::MetaSprite;

AbstractMsDocument::AbstractMsDocument(QObject* parent)
    : AbstractDocument(parent)
    , _animationListModel(new Animation::AnimationListModel(this))
    , _animationFramesManager(new Animation::AnimationFramesManager(this))
{
}

void AbstractMsDocument::initModels()
{
    _animationListModel->setDocument(this);
    _animationFramesManager->setDocument(this);
}
