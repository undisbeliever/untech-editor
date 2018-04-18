/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationdock.h"
#include "animationaccessors.h"
#include "animationactions.h"
#include "animationframesmanager.h"
#include "animationlistmodel.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/animation/animationdock.ui.h"
#include "gui-qt/undo/idmapundohelper.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationDock::AnimationDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::AnimationDock)
    , _actions(new AnimationActions(this))
    , _document(nullptr)
    , _animationListModel(new AnimationListModel(this))
    , _animationFramesManager(new AnimationFramesManager(this))
{
    _ui->setupUi(this);

    _ui->animationList->setModel(_animationListModel);

    QCompleter* nextAnimationCompleter = new QCompleter(_animationListModel, this);
    nextAnimationCompleter->setCompletionRole(Qt::DisplayRole);
    nextAnimationCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    _ui->nextAnimation->setCompleter(nextAnimationCompleter);
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

    _ui->animationFrames->setPropertyManager(_animationFramesManager);

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

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->animationsMap()->disconnect(this);

        _ui->animationList->selectionModel()->disconnect(this);
    }
    _document = document;

    _actions->setDocument(document);
    _animationListModel->setDocument(document);
    _animationFramesManager->setDocument(document);

    setEnabled(_document != nullptr);

    if (_document) {
        _ui->animationFrames->setColumnWidth(0, _ui->animationFrames->width() / 3);
        _ui->animationFrames->setColumnWidth(1, 45);
        _ui->animationFrames->setColumnWidth(2, 30);
        _ui->animationFrames->setColumnWidth(3, 0);

        onSelectedAnimationChanged();

        connect(_document->animationsMap(), &AnimationsMap::dataChanged,
                this, &AnimationDock::onAnimationDataChanged);

        connect(_document->animationsMap(), &AnimationsMap::selectedItemChanged,
                this, &AnimationDock::onSelectedAnimationChanged);

        connect(_ui->animationList->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &AnimationDock::onAnimationListSelectionChanged);
    }
    else {
        clearGui();
    }
}

void AnimationDock::onSelectedAnimationChanged()
{
    const MSA::Animation* ani = _document->animationsMap()->selectedItem();
    const idstring& id = _document->animationsMap()->selectedId();

    _ui->animationBox->setEnabled(ani != nullptr);

    if (ani) {
        updateGui();

        _ui->animationList->setCurrentIndex(
            _animationListModel->toModelIndex(id));
    }
    else {
        clearGui();
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

void AnimationDock::updateGui()
{
    const MSA::Animation* ani = _document->animationsMap()->selectedItem();

    _ui->durationFormat->setCurrentEnum(ani->durationFormat);
    _ui->oneShot->setChecked(ani->oneShot);

    _ui->nextAnimation->setEnabled(!ani->oneShot);
    _ui->nextAnimationLabel->setEnabled(!ani->oneShot);
    _ui->nextAnimation->setText(QString::fromStdString(ani->nextAnimation));
}

void AnimationDock::onDurationFormatEdited()
{
    MSA::DurationFormat df = _ui->durationFormat->currentEnum<MSA::DurationFormat>();

    AnimationUndoHelper helper(_document->animationsMap());
    helper.editSelectedItemField(df, tr("Change Animation Duration Format"),
                                 [](MSA::Animation& a) -> MSA::DurationFormat& { return a.durationFormat; });
}

void AnimationDock::onOneShotEdited()
{
    bool oneShot = _ui->oneShot->isChecked();

    QString text = oneShot ? tr("Set Animation One Shot") : tr("Clear Animation One Shot");

    AnimationUndoHelper helper(_document->animationsMap());
    helper.editSelectedItemField(oneShot, text,
                                 [](MSA::Animation& a) -> bool& { return a.oneShot; });
}

void AnimationDock::onNextAnimationEdited()
{
    idstring nextAni = _ui->nextAnimation->text().toStdString();

    AnimationUndoHelper helper(_document->animationsMap());
    helper.editSelectedItemField(nextAni, tr("Change Next Animation"),
                                 [](MSA::Animation& a) -> idstring& { return a.nextAnimation; });
}

void AnimationDock::onAnimationListSelectionChanged()
{
    if (_document) {
        QModelIndex index = _ui->animationList->currentIndex();
        idstring id = _animationListModel->toIdstring(index);
        _document->animationsMap()->setSelectedId(id);
    }
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
