/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationactions.h"
#include "animationcommands.h"
#include "animationframecommands.h"
#include "animationlistmodel.h"
#include "gui-qt/common/idstringdialog.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/abstractselection.h"

using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationActions::AnimationActions(QWidget* widget)
    : QObject(widget)
    , _widget(widget)
    , _document(nullptr)
    , _addAnimation(new QAction(QIcon(":/icons/add.svg"), tr("New Animation"), this))
    , _cloneAnimation(new QAction(QIcon(":/icons/clone.svg"), tr("Clone Animation"), this))
    , _renameAnimation(new QAction(QIcon(":/icons/rename.svg"), tr("Rename Animation"), this))
    , _removeAnimation(new QAction(QIcon(":/icons/remove.svg"), tr("Remove Animation"), this))
{
    updateActions();

    connect(_addAnimation, &QAction::triggered,
            this, &AnimationActions::onAddAnimation);
    connect(_cloneAnimation, &QAction::triggered,
            this, &AnimationActions::onCloneAnimation);
    connect(_renameAnimation, &QAction::triggered,
            this, &AnimationActions::onRenameAnimation);
    connect(_removeAnimation, &QAction::triggered,
            this, &AnimationActions::onRemoveAnimation);
}

void AnimationActions::setDocument(AbstractMsDocument* document)
{
    if (_document) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    if (document) {
        connect(_document, &AbstractMsDocument::animationMapChanged,
                this, &AnimationActions::updateActions);
        connect(_document, &AbstractMsDocument::animationFrameListChanged,
                this, &AnimationActions::updateActions);
        connect(_document->selection(), &AbstractSelection::selectedAnimationChanged,
                this, &AnimationActions::updateActions);
    }

    updateActions();
}

void AnimationActions::updateActions()
{
    bool documentExists = false;
    bool animationSelected = false;

    if (_document) {
        documentExists = true;

        const auto* sel = _document->selection();
        animationSelected = sel->selectedAnimation();
    }

    _addAnimation->setEnabled(documentExists);
    _cloneAnimation->setEnabled(animationSelected);
    _renameAnimation->setEnabled(animationSelected);
    _removeAnimation->setEnabled(animationSelected);
}

void AnimationActions::onAddAnimation()
{
    const MSA::Animation::map_t* animations = _document->animations();

    idstring newId = IdstringDialog::getIdstring(
        _widget,
        tr("Input Animation Name"),
        tr("Input name of the new animation:"),
        idstring(), _document->animationListModel());

    if (newId.isValid() && !animations->contains(newId)) {
        _document->undoStack()->push(
            new AddAnimation(_document, newId));

        _document->selection()->selectAnimation(newId);
    }
}

void AnimationActions::onCloneAnimation()
{
    const MSA::Animation::map_t* animations = _document->animations();
    const idstring& animationId = _document->selection()->selectedAnimationId();

    idstring newId = IdstringDialog::getIdstring(
        _widget,
        tr("Input Animation Name"),
        tr("Input name of the cloned animation:"),
        animationId, _document->animationListModel());

    if (newId != animationId && newId.isValid() && !animations->contains(newId)) {
        _document->undoStack()->push(
            new CloneAnimation(_document, animationId, newId));

        _document->selection()->selectAnimation(newId);
    }
}

void AnimationActions::onRenameAnimation()
{
    const MSA::Animation::map_t* animations = _document->animations();
    const idstring& animationId = _document->selection()->selectedAnimationId();

    idstring newId = IdstringDialog::getIdstring(
        _widget,
        tr("Input Animation Name"),
        tr("Rename %1 to:").arg(QString::fromStdString(animationId)),
        animationId, _document->animationListModel());

    if (newId != animationId && newId.isValid() && !animations->contains(newId)) {
        _document->undoStack()->push(
            new RenameAnimation(_document, animationId, newId));
    }
}

void AnimationActions::onRemoveAnimation()
{
    idstring animationId = _document->selection()->selectedAnimationId();

    _document->undoStack()->push(
        new RemoveAnimation(_document, animationId));
}
