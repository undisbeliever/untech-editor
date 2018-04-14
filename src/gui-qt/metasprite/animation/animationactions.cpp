/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationactions.h"
#include "animationaccessors.h"
#include "animationlistmodel.h"
#include "gui-qt/common/idstringdialog.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/abstractselection.h"
#include "gui-qt/undo/idmapundohelper.h"

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
        _document->animationsMap()->disconnect(this);
    }
    _document = document;

    if (document) {
        connect(_document->animationsMap(), &AnimationsMap::mapChanged,
                this, &AnimationActions::updateActions);
        connect(_document->animationsMap(), &AnimationsMap::selectedItemChanged,
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

        animationSelected = _document->animationsMap()->selectedItem() != nullptr;
    }

    _addAnimation->setEnabled(documentExists);
    _cloneAnimation->setEnabled(animationSelected);
    _renameAnimation->setEnabled(animationSelected);
    _removeAnimation->setEnabled(animationSelected);
}

void AnimationActions::onAddAnimation()
{
    idstring newId = IdstringDialog::getIdstring(
        _widget,
        tr("Input Animation Name"),
        tr("Input name of the new animation:"),
        idstring(), _document->animationsMap()->animationNames());

    AnimationUndoHelper(_document->animationsMap()).addItem(newId);
}

void AnimationActions::onCloneAnimation()
{
    const idstring& animationId = _document->animationsMap()->selectedId();

    idstring newId = IdstringDialog::getIdstring(
        _widget,
        tr("Input Animation Name"),
        tr("Input name of the cloned animation:"),
        animationId, _document->animationsMap()->animationNames());

    AnimationUndoHelper(_document->animationsMap()).cloneSelectedItem(newId);
}

void AnimationActions::onRenameAnimation()
{
    const idstring& animationId = _document->animationsMap()->selectedId();

    idstring newId = IdstringDialog::getIdstring(
        _widget,
        tr("Input Animation Name"),
        tr("Rename %1 to:").arg(QString::fromStdString(animationId)),
        animationId, _document->animationsMap()->animationNames());

    AnimationUndoHelper(_document->animationsMap()).renameSelectedItem(newId);
}

void AnimationActions::onRemoveAnimation()
{
    AnimationUndoHelper(_document->animationsMap()).removeSelectedItem();
}
