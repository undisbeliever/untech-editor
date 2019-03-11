/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationdock.h"
#include "animationaccessors.h"
#include "managers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/animation/animationdock.ui.h"

#include <QCompleter>
#include <QMenu>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationDock::AnimationDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::AnimationDock)
    , _document(nullptr)
    , _animationManager(new AnimationManager(this))
    , _animationFramesManager(new AnimationFramesManager(this))
{
    _ui->setupUi(this);

    _ui->animationList->namedListActions().populate(_ui->animationListButtons);

    _ui->animationProperties->setPropertyManager(_animationManager);

    _ui->animationFrames->setPropertyManager(_animationFramesManager);
    _ui->animationFrames->viewActions()->populate(_ui->animationFramesButtons);

    setEnabled(false);
}

AnimationDock::~AnimationDock() = default;

const UnTech::GuiQt::Accessor::NamedListActions& AnimationDock::actions() const
{
    return _ui->animationList->namedListActions();
}

Accessor::NamedListModel* AnimationDock::animationListModel()
{
    return _ui->animationList->namedListModel();
}

void AnimationDock::setDocument(AbstractMsDocument* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->animationsList()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    AnimationsList* animationsList = _document ? _document->animationsList() : nullptr;

    _animationManager->setDocument(document);
    _animationFramesManager->setDocument(document);
    _ui->animationList->setAccessor(animationsList);

    if (_document) {
        _ui->animationFrames->setColumnWidth(0, _ui->animationFrames->width() / 3);
        _ui->animationFrames->setColumnWidth(1, 45);
        _ui->animationFrames->setColumnWidth(2, 30);
        _ui->animationFrames->setColumnWidth(3, 0);
    }
}
