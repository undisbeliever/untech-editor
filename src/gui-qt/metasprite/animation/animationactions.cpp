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
{
    _addAnimation = new QAction(QIcon(":/icons/add.svg"), tr("New Animation"), this);
    _cloneAnimation = new QAction(QIcon(":/icons/clone.svg"), tr("Clone Animation"), this);
    _renameAnimation = new QAction(QIcon(":/icons/rename.svg"), tr("Rename Animation"), this);
    _removeAnimation = new QAction(QIcon(":/icons/remove.svg"), tr("Remove Animation"), this);

    _addAnimationFrame = new QAction(QIcon(":icons/add.svg"), tr("New Animation Frame"), this);
    _raiseAnimationFrame = new QAction(QIcon(":icons/raise.svg"), tr("Raise Animation Frame"), this);
    _lowerAnimationFrame = new QAction(QIcon(":icons/lower.svg"), tr("Lower Animation Frame"), this);
    _cloneAnimationFrame = new QAction(QIcon(":icons/clone.svg"), tr("Clone Animation Frame"), this);
    _removeAnimationFrame = new QAction(QIcon(":icons/remove.svg"), tr("Remove Animation Frame"), this);

    updateActions();

    connect(_addAnimation, &QAction::triggered,
            this, &AnimationActions::onAddAnimation);
    connect(_cloneAnimation, &QAction::triggered,
            this, &AnimationActions::onCloneAnimation);
    connect(_renameAnimation, &QAction::triggered,
            this, &AnimationActions::onRenameAnimation);
    connect(_removeAnimation, &QAction::triggered,
            this, &AnimationActions::onRemoveAnimation);

    connect(_addAnimationFrame, &QAction::triggered,
            this, &AnimationActions::onAddAnimationFrame);
    connect(_raiseAnimationFrame, &QAction::triggered,
            this, &AnimationActions::onRaiseAnimationFrame);
    connect(_lowerAnimationFrame, &QAction::triggered,
            this, &AnimationActions::onLowerAnimationFrame);
    connect(_cloneAnimationFrame, &QAction::triggered,
            this, &AnimationActions::onCloneAnimationFrame);
    connect(_removeAnimationFrame, &QAction::triggered,
            this, &AnimationActions::onRemoveAnimationFrame);
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
        connect(_document->selection(), &AbstractSelection::selectedAnimationFrameChanged,
                this, &AnimationActions::updateActions);
    }

    updateActions();
}

void AnimationActions::updateActions()
{
    bool documentExists = false;
    bool animationSelected = false;
    bool animationFrameSelected = false;
    bool canInsertAnimationFrame = false;
    bool canRaiseAnimationFrame = false;
    bool canLowerAnimationFrame = false;

    if (_document) {
        documentExists = true;

        const auto* sel = _document->selection();

        if (const MSA::Animation* animation = sel->selectedAnimation()) {
            animationSelected = true;

            int fIndex = sel->selectedAnimationFrame();
            int nFrames = animation->frames.size();

            canInsertAnimationFrame = animation->frames.can_insert();
            animationFrameSelected = fIndex >= 0 && fIndex < nFrames;
            canRaiseAnimationFrame = fIndex >= 1 && fIndex < nFrames;
            canLowerAnimationFrame = fIndex >= 0 && fIndex + 1 < nFrames;
        }
    }

    _addAnimation->setEnabled(documentExists);
    _cloneAnimation->setEnabled(animationSelected);
    _renameAnimation->setEnabled(animationSelected);
    _removeAnimation->setEnabled(animationSelected);

    _addAnimationFrame->setEnabled(canInsertAnimationFrame);
    _raiseAnimationFrame->setEnabled(canRaiseAnimationFrame);
    _lowerAnimationFrame->setEnabled(canLowerAnimationFrame);
    _cloneAnimationFrame->setEnabled(animationFrameSelected && canInsertAnimationFrame);
    _removeAnimationFrame->setEnabled(animationFrameSelected);
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

void AnimationActions::onAddAnimationFrame()
{
    MSA::Animation* animation = _document->selection()->selectedAnimation();

    _document->undoStack()->push(
        new AddAnimationFrame(_document, animation));

    _document->selection()->selectAnimationFrame(animation->frames.size() - 1);
}

void AnimationActions::onRaiseAnimationFrame()
{
    MSA::Animation* animation = _document->selection()->selectedAnimation();
    unsigned index = _document->selection()->selectedAnimationFrame();

    _document->undoStack()->push(
        new RaiseAnimationFrame(_document, animation, index));

    _document->selection()->selectAnimationFrame(index - 1);
}

void AnimationActions::onLowerAnimationFrame()
{
    MSA::Animation* animation = _document->selection()->selectedAnimation();
    unsigned index = _document->selection()->selectedAnimationFrame();

    _document->undoStack()->push(
        new LowerAnimationFrame(_document, animation, index));

    _document->selection()->selectAnimationFrame(index + 1);
}

void AnimationActions::onCloneAnimationFrame()
{
    MSA::Animation* animation = _document->selection()->selectedAnimation();
    unsigned index = _document->selection()->selectedAnimationFrame();

    _document->undoStack()->push(
        new CloneAnimationFrame(_document, animation, index));

    _document->selection()->selectAnimationFrame(animation->frames.size() - 1);
}

void AnimationActions::onRemoveAnimationFrame()
{
    MSA::Animation* animation = _document->selection()->selectedAnimation();
    unsigned index = _document->selection()->selectedAnimationFrame();

    _document->undoStack()->push(
        new RemoveAnimationFrame(_document, animation, index));
}
