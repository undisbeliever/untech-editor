/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationdock.h"
#include "animationframesmodel.h"
#include "animationlistmodel.h"
#include "gui-qt/metasprite/abstractdocument.h"
#include "gui-qt/metasprite/abstractselection.h"
#include "gui-qt/metasprite/animation/animationdock.ui.h"

using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationDock::AnimationDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::AnimationDock)
    , _document(nullptr)
{
    _ui->setupUi(this);

    _ui->durationFormat->populateData(MSA::DurationFormat::enumMap);

    clearGui();
    setEnabled(false);
}

AnimationDock::~AnimationDock() = default;

void AnimationDock::setDocument(AbstractDocument* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        if (auto* m = _ui->animationList->selectionModel()) {
            m->deleteLater();
        }
        if (auto* m = _ui->animationFrames->selectionModel()) {
            m->deleteLater();
        }

        _ui->animationList->setModel(_document->animationListModel());
        _ui->animationFrames->setModel(_document->animationFramesModel());

        _ui->animationFrames->setColumnWidth(0, _ui->animationFrames->width() / 2);
        _ui->animationFrames->setColumnWidth(1, 45);
        _ui->animationFrames->setColumnWidth(2, 0);

        onSelectedAnimationChanged();
        onSelectedAnimationFrameChanged();

        connect(_document->selection(), &AbstractSelection::selectedAnimationChanged,
                this, &AnimationDock::onSelectedAnimationChanged);

        connect(_document->selection(), &AbstractSelection::selectedAnimationFrameChanged,
                this, &AnimationDock::onSelectedAnimationFrameChanged);

        connect(_ui->animationList->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &AnimationDock::onAnimationListSelectionChanged);

        connect(_ui->animationFrames->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &AnimationDock::onAnimationFrameSelectionChanged);
    }
    else {
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

void AnimationDock::onSelectedAnimationFrameChanged()
{
    AnimationFramesModel* model = _document->animationFramesModel();
    unsigned pos = _document->selection()->selectedAnimationFrame();

    _ui->animationFrames->setCurrentIndex(model->toModelIndex(pos));
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

void AnimationDock::onAnimationListSelectionChanged()
{
    QModelIndex index = _ui->animationList->currentIndex();
    idstring id = _document->animationListModel()->toIdstring(index);
    _document->selection()->selectAnimation(id);
}

void AnimationDock::onAnimationFrameSelectionChanged()
{
    QModelIndex index = _ui->animationFrames->currentIndex();
    _document->selection()->selectAnimationFrame(index.row());
}
