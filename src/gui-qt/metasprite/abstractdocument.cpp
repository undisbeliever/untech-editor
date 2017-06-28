/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractdocument.h"
#include "animation/animationframesmodel.h"
#include "animation/animationlistmodel.h"

using namespace UnTech::GuiQt::MetaSprite;

AbstractDocument::AbstractDocument(QObject* parent)
    : QObject(parent)
    , _filename()
    , _undoStack(new QUndoStack(this))
    , _animationListModel(new Animation::AnimationListModel(this))
    , _animationFramesModel(new Animation::AnimationFramesModel(this))
{
}

void AbstractDocument::initModels()
{
    _animationListModel->setDocument(this);
    _animationFramesModel->setDocument(this);
}
