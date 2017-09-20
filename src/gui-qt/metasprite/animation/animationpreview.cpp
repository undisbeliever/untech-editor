/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationpreview.h"
#include "animationlistmodel.h"
#include "animationpreviewitem.h"
#include "gui-qt/common/graphics/zoomsettings.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "gui-qt/metasprite/abstractselection.h"
#include "gui-qt/metasprite/animation/animationpreview.ui.h"

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationPreview::AnimationPreview(QWidget* parent)
    : QWidget(parent)
    , _ui(new Ui::AnimationPreview)
    , _graphicsScene(new QGraphicsScene(this))
    , _itemFactory(nullptr)
    , _zoomSettings(nullptr)
    , _document(nullptr)
    , _previewItem(nullptr)
    , _timer()
    , _elapsed()
    , _nsSinceLastFrame()
{
    _ui->setupUi(this);

    _ui->graphicsView->setScene(_graphicsScene);

    _ui->region->addItem("NTSC", 1000000000 / 60);
    _ui->region->addItem("PAL", 1000000000 / 50);

    _elapsed.start();

    updateGui();
    updateSceneRect();

    setEnabled(false);

    clearGui();

    connect(_ui->playButton, &QToolButton::toggled,
            this, &AnimationPreview::updateTimer);
    connect(_ui->stepButton, &QToolButton::clicked,
            this, &AnimationPreview::onStepClicked);
    connect(_ui->skipButton, &QToolButton::clicked,
            this, &AnimationPreview::onSkipClicked);
    connect(_ui->resetButton, &QToolButton::clicked,
            this, &AnimationPreview::onResetClicked);

    connect(_ui->region, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &AnimationPreview::onRegionChanged);

    connect(_ui->animation, qOverload<int>(&QComboBox::activated),
            this, &AnimationPreview::onAnimationComboActivated);
    connect(_ui->xVelocity, &QSlider::valueChanged,
            this, &AnimationPreview::onVelocityChanged);
    connect(_ui->yVelocity, &QSlider::valueChanged,
            this, &AnimationPreview::onVelocityChanged);

    connect(_ui->zeroXVelocityButton, &QToolButton::clicked,
            this, &AnimationPreview::onZeroXVelocityClicked);
    connect(_ui->zeroYVelocityButton, &QToolButton::clicked,
            this, &AnimationPreview::onZeroYVelocityClicked);
}

AnimationPreview::~AnimationPreview() = default;

void AnimationPreview::setItemFactory(AnimationPreviewItemFactory* itemFactory)
{
    Q_ASSERT(itemFactory != nullptr);

    removePreviewItem();

    _itemFactory = itemFactory;
}

void AnimationPreview::setZoomSettings(ZoomSettings* zoomSettings)
{
    if (_zoomSettings) {
        _zoomSettings->disconnect(this);
    }
    _zoomSettings = zoomSettings;

    _ui->graphicsView->setZoomSettings(zoomSettings);
    updateSceneRect();

    connect(_zoomSettings, &ZoomSettings::transformChanged,
            this, &AnimationPreview::updateSceneRect);
}

void AnimationPreview::setDocument(AbstractMsDocument* document)
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
        _ui->animation->setModel(_document->animationListModel());

        onSelectedAnimationChanged();

        connect(_document, &AbstractMsDocument::animationFrameChanged,
                this, &AnimationPreview::onAnimationFramesChanged);
        connect(_document, &AbstractMsDocument::animationFrameListChanged,
                this, &AnimationPreview::onAnimationFramesChanged);

        connect(_document->selection(), &AbstractSelection::selectedAnimationChanged,
                this, &AnimationPreview::onSelectedAnimationChanged);
    }
    else {
        removePreviewItem();
        clearGui();
        _ui->animation->setModel(nullptr);
    }
}

void AnimationPreview::removePreviewItem()
{
    stopTimer();
    clearGui();

    _previewItem = nullptr;
    _graphicsScene->clear();
}

void AnimationPreview::createPreviewItem()
{
    if (_previewItem != nullptr) {
        return;
    }
    if (_itemFactory == nullptr || _document == nullptr) {
        return;
    }

    _previewItem = _itemFactory->createPreviewItem(_document);
    _graphicsScene->addItem(_previewItem);

    onRegionChanged();
    onVelocityChanged();

    _previewItem->setAnimation(_document->selection()->selectedAnimationId());

    updateGui();
}

void AnimationPreview::clearGui()
{
    _ui->animation->setCurrentIndex(-1);

    _ui->xVelocity->setValue(0);
    _ui->yVelocity->setValue(0);

    _ui->displayFrame->setText(QString());
    _ui->animationFrame->setText(QString());
    _ui->metaSpriteFrame->setText(QString());
}

void AnimationPreview::updateGui()
{
    if (_previewItem == nullptr) {
        clearGui();
        return;
    }

    const MSA::PreviewState& state = _previewItem->state();

    QString animationFrame;
    if (state.isRunning()) {
        animationFrame = QString::fromStdString(state.animationId())
                         + "." + QString::number(state.animationFrameIndex());
    }

    _ui->displayFrame->setNum((int)state.displayFrameCount());
    _ui->animationFrame->setText(animationFrame);
    _ui->metaSpriteFrame->setText(QString::fromStdString(state.frame().str()));
}

void AnimationPreview::updateSceneRect()
{
    const QSize s = _ui->graphicsView->size();
    QRect rect(-s.width() / 2, -s.height() / 2, s.width(), s.height());

    // There is no QGraphicsView::transformChanged signal and I am unsure
    // if the ZoomableGraphicsView::onZoomSettingsChanged slot is called
    // before this function so I must get the transform from _zoomSettings.
    if (_zoomSettings) {
        rect = _zoomSettings->transform().inverted().mapRect(rect);
    }
    _graphicsScene->setSceneRect(rect);

    if (_previewItem) {
        _previewItem->sync();
    }
}

void AnimationPreview::onSelectedAnimationChanged()
{
    const idstring& id = _document->selection()->selectedAnimationId();

    _ui->animation->setCurrentIndex(
        _document->animationListModel()->toModelIndex(id).row());

    if (id.isValid()) {
        if (_previewItem == nullptr) {
            createPreviewItem();
        }
        else {
            _previewItem->setAnimation(id);
            updateGui();
        }
    }
    else {
        removePreviewItem();
    }
}

void AnimationPreview::onAnimationFramesChanged()
{
    if (_previewItem != nullptr) {
        _previewItem->sync();
        updateGui();
    }
}

void AnimationPreview::onAnimationComboActivated()
{
    const idstring id = _ui->animation->currentText().toStdString();
    _document->selection()->selectAnimation(id);
}

void AnimationPreview::onVelocityChanged()
{
    const qreal DIVISOR = 1 << MSA::PreviewState::FP_SHIFT;

    if (_previewItem) {
        _previewItem->setVelocityFp(
            point(_ui->xVelocity->value(), _ui->yVelocity->value()));
    }

    _ui->xVelocity->setToolTip(tr("%1 px/frame").arg(_ui->xVelocity->value() / DIVISOR));
    _ui->yVelocity->setToolTip(tr("%1 px/frame").arg(_ui->yVelocity->value() / DIVISOR));
}

void AnimationPreview::onRegionChanged()
{
    using Region = MSA::PreviewState::Region;

    if (_previewItem) {
        unsigned nsPerFrame = _ui->region->currentData().toUInt();
        if (nsPerFrame == 1000000000 / 60) {
            _previewItem->setRegion(Region::NTSC);
        }
        else {
            _previewItem->setRegion(Region::PAL);
        }
    }

    updateTimer();
}

void AnimationPreview::onZeroXVelocityClicked()
{
    _ui->xVelocity->setValue(0);
}

void AnimationPreview::onZeroYVelocityClicked()
{
    _ui->yVelocity->setValue(0);
}

void AnimationPreview::onStepClicked()
{
    stopTimer();

    if (_previewItem) {
        _previewItem->processDisplayFrame();
    }

    updateGui();
}

void AnimationPreview::onSkipClicked()
{
    stopTimer();

    if (_previewItem) {
        _previewItem->nextAnimationFrame();
    }

    updateGui();
}

void AnimationPreview::onResetClicked()
{
    stopTimer();

    if (_previewItem) {
        _previewItem->resetAnimation();
    }

    updateGui();
}

void AnimationPreview::updateTimer()
{
    if (_ui->playButton->isChecked() && _ui->region->currentIndex() >= 0) {
        _elapsed.restart();
        _nsSinceLastFrame = _elapsed.nsecsElapsed();
        _timer.start(1, Qt::PreciseTimer, this);
    }
    else {
        _timer.stop();
    }
}

void AnimationPreview::stopTimer()
{
    _ui->playButton->setChecked(false);
    _timer.stop();
}

void AnimationPreview::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == _timer.timerId()) {
        if (_previewItem) {
            unsigned nsPerFrame = _ui->region->currentData().toUInt();
            qint64 nsecsElapsed = _elapsed.nsecsElapsed();
            qint64 ns = nsecsElapsed - _nsSinceLastFrame;

            if (ns > nsPerFrame * 4) {
                ns = nsPerFrame * 4;
                _nsSinceLastFrame = nsecsElapsed - ns;
            }
            while (ns >= nsPerFrame) {
                ns -= nsPerFrame;
                _nsSinceLastFrame += nsPerFrame;

                _previewItem->processDisplayFrame();
                if (_previewItem->state().isRunning() == false) {
                    stopTimer();
                    break;
                }
            }
        }
        else {
            stopTimer();
        }
        updateGui();
    }
    else {
        QWidget::timerEvent(event);
    }
}

void AnimationPreview::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    updateSceneRect();
}

void AnimationPreview::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);

    stopTimer();
}
