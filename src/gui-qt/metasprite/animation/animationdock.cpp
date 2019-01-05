/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationdock.h"
#include "animationaccessors.h"
#include "animationframesmanager.h"
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
    , _animationFramesManager(new AnimationFramesManager(this))
{
    _ui->setupUi(this);

    QCompleter* nextAnimationCompleter = new QCompleter(_ui->animationList->model(), this);
    nextAnimationCompleter->setCompletionRole(Qt::DisplayRole);
    nextAnimationCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    _ui->nextAnimation->setCompleter(nextAnimationCompleter);
    _ui->nextAnimation->setValidator(new IdstringValidator(this));

    _ui->durationFormat->populateData(MSA::DurationFormat::enumMap);

    _ui->animationList->idmapActions().populateToolbar(_ui->animationListButtons);

    _ui->animationFramesButtons->addAction(_ui->animationFrames->insertAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->cloneAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->raiseToTopAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->raiseAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->lowerAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->lowerToBottomAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->cloneAction());
    _ui->animationFramesButtons->addAction(_ui->animationFrames->removeAction());

    _ui->animationFrames->setPropertyManager(_animationFramesManager);

    clearGui();
    setEnabled(false);

    connect(_ui->durationFormat, qOverload<int>(&QComboBox::activated),
            this, &AnimationDock::onDurationFormatEdited);
    connect(_ui->oneShot, &QCheckBox::clicked,
            this, &AnimationDock::onOneShotEdited);
    connect(_ui->nextAnimation, &QLineEdit::editingFinished,
            this, &AnimationDock::onNextAnimationEdited);
}

AnimationDock::~AnimationDock() = default;

const UnTech::GuiQt::Accessor::IdmapActions& AnimationDock::actions() const
{
    return _ui->animationList->idmapActions();
}

Accessor::IdmapListModel* AnimationDock::animationListModel()
{
    return _ui->animationList->idmapListModel();
}

void AnimationDock::setDocument(AbstractMsDocument* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->animationsMap()->disconnect(this);
    }
    _document = document;

    _animationFramesManager->setDocument(document);

    setEnabled(_document != nullptr);

    if (_document) {
        updateGui();

        _ui->animationList->setAccessor(_document->animationsMap());

        _ui->animationFrames->setColumnWidth(0, _ui->animationFrames->width() / 3);
        _ui->animationFrames->setColumnWidth(1, 45);
        _ui->animationFrames->setColumnWidth(2, 30);
        _ui->animationFrames->setColumnWidth(3, 0);

        connect(_document->animationsMap(), &AnimationsMap::selectedItemChanged,
                this, &AnimationDock::updateGui);
        connect(_document->animationsMap(), &AnimationsMap::dataChanged,
                this, &AnimationDock::onAnimationDataChanged);
    }
    else {
        clearGui();
        _ui->animationList->setAccessor<AnimationsMap>(nullptr);
    }
}

void AnimationDock::onAnimationDataChanged(const void* animation)
{
    if (animation == _document->animationsMap()->selectedItem()) {
        updateGui();
    }
}

void AnimationDock::clearGui()
{
    _ui->durationFormat->setCurrentIndex(-1);
    _ui->oneShot->setChecked(false);
    _ui->nextAnimation->clear();
}

void AnimationDock::selectAnimationFrame(unsigned index)
{
    _ui->animationFrames->setSelectedRow(_animationFramesManager, index);
}

void AnimationDock::updateGui()
{
    Q_ASSERT(_document);

    const MSA::Animation* ani = _document->animationsMap()->selectedItem();
    if (ani) {
        _ui->durationFormat->setCurrentEnum(ani->durationFormat);
        _ui->oneShot->setChecked(ani->oneShot);

        _ui->nextAnimation->setEnabled(!ani->oneShot);
        _ui->nextAnimationLabel->setEnabled(!ani->oneShot);
        _ui->nextAnimation->setText(QString::fromStdString(ani->nextAnimation));
    }
    else {
        clearGui();
    }
}

void AnimationDock::onDurationFormatEdited()
{
    _document->animationsMap()->editSelected_setDurationFormat(
        _ui->durationFormat->currentEnum<MSA::DurationFormat>());
}

void AnimationDock::onOneShotEdited()
{
    _document->animationsMap()->editSelected_setOneShot(
        _ui->oneShot->isChecked());
}

void AnimationDock::onNextAnimationEdited()
{
    _document->animationsMap()->editSelected_setNextAnimation(
        _ui->nextAnimation->text().toStdString());
}
