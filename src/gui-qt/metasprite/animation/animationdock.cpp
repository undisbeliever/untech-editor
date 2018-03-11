/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationdock.h"
#include "animationactions.h"
#include "animationcommands.h"
#include "animationframesmanager.h"
#include "animationlistmodel.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/abstractselection.h"
#include "gui-qt/metasprite/animation/animationdock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationDock::AnimationDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::AnimationDock)
    , _actions(new AnimationActions(this))
    , _document(nullptr)
    , _nextAnimationCompleter(new QCompleter(this))
{
    _ui->setupUi(this);

    _nextAnimationCompleter->setCompletionRole(Qt::DisplayRole);
    _nextAnimationCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    _ui->nextAnimation->setCompleter(_nextAnimationCompleter);
    _ui->nextAnimation->setValidator(new IdstringValidator(this));

    _ui->durationFormat->populateData(MSA::DurationFormat::enumMap);

    _ui->animationListButtons->addAction(_actions->addAnimation());
    _ui->animationListButtons->addAction(_actions->cloneAnimation());
    _ui->animationListButtons->addAction(_actions->renameAnimation());
    _ui->animationListButtons->addAction(_actions->removeAnimation());

    _ui->animationFramesButtons->addAction(_ui->animationFrames->insertAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->cloneAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->raiseToTopAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->raiseAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->lowerAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->lowerToBottomAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->cloneAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->removeAction());

    _ui->animationList->setContextMenuPolicy(Qt::CustomContextMenu);

    clearGui();
    setEnabled(false);

    connect(_ui->durationFormat, qOverload<int>(&QComboBox::activated),
            this, &AnimationDock::onDurationFormatEdited);
    connect(_ui->oneShot, &QCheckBox::clicked,
            this, &AnimationDock::onOneShotEdited);
    connect(_ui->nextAnimation, &QLineEdit::editingFinished,
            this, &AnimationDock::onNextAnimationEdited);

    connect(_ui->animationList, &QListView::customContextMenuRequested,
            this, &AnimationDock::onAnimationListContextMenu);
}

AnimationDock::~AnimationDock() = default;

void AnimationDock::setDocument(AbstractMsDocument* document)
{
    if (_document == document) {
        return;
    }

    if (auto* m = _ui->animationList->selectionModel()) {
        m->deleteLater();
    }
    if (auto* m = _ui->animationFrames->selectionModel()) {
        m->deleteLater();
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    _actions->setDocument(document);

    setEnabled(_document != nullptr);

    if (_document) {
        _nextAnimationCompleter->setModel(_document->animationListModel());
        _ui->animationList->setModel(_document->animationListModel());
        _ui->animationFrames->setPropertyManager(_document->animationFramesManager());

        _ui->animationFrames->setColumnWidth(0, _ui->animationFrames->width() / 3);
        _ui->animationFrames->setColumnWidth(1, 45);
        _ui->animationFrames->setColumnWidth(2, 30);
        _ui->animationFrames->setColumnWidth(3, 0);

        onSelectedAnimationChanged();

        connect(_document, &AbstractMsDocument::animationDataChanged,
                this, &AnimationDock::onAnimationDataChanged);

        connect(_document->selection(), &AbstractSelection::selectedAnimationChanged,
                this, &AnimationDock::onSelectedAnimationChanged);

        connect(_ui->animationList->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &AnimationDock::onAnimationListSelectionChanged);
    }
    else {
        _nextAnimationCompleter->setModel(nullptr);
        _ui->animationList->setModel(nullptr);
        _ui->animationFrames->setModel(nullptr);

        clearGui();
    }
}

void AnimationDock::onSelectedAnimationChanged()
{
    MSA::Animation* ani = _document->selection()->selectedAnimation();
    const idstring& id = _document->selection()->selectedAnimationId();

    _ui->animationBox->setEnabled(ani != nullptr);

    if (ani) {
        updateGui();

        _ui->animationList->setCurrentIndex(
            _document->animationListModel()->toModelIndex(id));
    }
    else {
        clearGui();
    }
}

void AnimationDock::onAnimationDataChanged(const void* animation)
{
    if (animation == _document->selection()->selectedAnimation()) {
        updateGui();
    }
}

void AnimationDock::clearGui()
{
    _ui->durationFormat->setCurrentIndex(-1);
    _ui->oneShot->setChecked(false);
    _ui->nextAnimation->clear();
}

void AnimationDock::updateGui()
{
    const MSA::Animation* ani = _document->selection()->selectedAnimation();

    _ui->durationFormat->setCurrentEnum(ani->durationFormat);
    _ui->oneShot->setChecked(ani->oneShot);

    _ui->nextAnimation->setEnabled(!ani->oneShot);
    _ui->nextAnimationLabel->setEnabled(!ani->oneShot);
    _ui->nextAnimation->setText(QString::fromStdString(ani->nextAnimation));
}

void AnimationDock::onDurationFormatEdited()
{
    MSA::Animation* ani = _document->selection()->selectedAnimation();

    MSA::DurationFormat df = _ui->durationFormat->currentEnum<MSA::DurationFormat>();
    if (df != ani->durationFormat) {
        _document->undoStack()->push(
            new ChangeAnimationDurationFormat(_document, ani, df));
    }
}

void AnimationDock::onOneShotEdited()
{
    MSA::Animation* ani = _document->selection()->selectedAnimation();

    bool oneShot = _ui->oneShot->isChecked();
    if (oneShot != ani->oneShot) {
        _document->undoStack()->push(
            new ChangeAnimationOneShot(_document, ani, oneShot));
    }
}

void AnimationDock::onNextAnimationEdited()
{
    MSA::Animation* ani = _document->selection()->selectedAnimation();

    idstring nextAni = _ui->nextAnimation->text().toStdString();
    if (nextAni != ani->nextAnimation) {
        _document->undoStack()->push(
            new ChangeAnimationNextAnimation(_document, ani, nextAni));
    }
}

void AnimationDock::onAnimationListSelectionChanged()
{
    QModelIndex index = _ui->animationList->currentIndex();
    idstring id = _document->animationListModel()->toIdstring(index);
    _document->selection()->selectAnimation(id);
}

void AnimationDock::onAnimationListContextMenu(const QPoint& pos)
{
    if (_document) {
        bool onAnimation = _ui->animationList->indexAt(pos).isValid();

        QMenu menu;
        menu.addAction(_actions->addAnimation());

        if (onAnimation) {
            menu.addAction(_actions->cloneAnimation());
            menu.addAction(_actions->renameAnimation());
            menu.addAction(_actions->removeAnimation());
        }

        QPoint globalPos = _ui->animationList->mapToGlobal(pos);
        menu.exec(globalPos);
    }
}
