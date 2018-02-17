/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetcentralwidget.h"
#include "mttilesetresourceitem.h"
#include "gui-qt/common/graphics/qpenhelper.h"
#include "gui-qt/common/graphics/zoomsettings.h"
#include "gui-qt/resources/mttileset/mttilesetcentralwidget.ui.h"

using namespace UnTech::GuiQt::Resources;

const QColor MtTilesetGraphicsItem::GRID_COLOR = QColor(200, 200, 255, 128);

MtTilesetCentralWidget::MtTilesetCentralWidget(QWidget* parent,
                                               ZoomSettings* zoomSettings)
    : AbstractResourceWidget(parent)
    , _ui(new Ui::MtTilesetCentralWidget)
    , _graphicsScene(new QGraphicsScene(this))
    , _tileset(nullptr)
    , _graphicsItem(nullptr)
{
    _ui->setupUi(this);
    _ui->graphicsView->setZoomSettings(zoomSettings);

    _animationTimer.setRegionCombo(_ui->region);
    _animationTimer.setPlayButton(_ui->playButton);

    _ui->graphicsView->setScene(_graphicsScene);

    setEnabled(false);

    connect(&_animationTimer, &AnimationTimer::animationFrameAdvance,
            this, &MtTilesetCentralWidget::onAnimationFrameAdvance);
    connect(_ui->previousButton, &QAbstractButton::clicked,
            this, &MtTilesetCentralWidget::onPreviousClicked);
    connect(_ui->nextButton, &QAbstractButton::clicked,
            this, &MtTilesetCentralWidget::onNextClicked);
}

MtTilesetCentralWidget::~MtTilesetCentralWidget() = default;

ResourceTypeIndex MtTilesetCentralWidget::resourceTypeIndex() const
{
    return ResourceTypeIndex::MT_TILESET;
}

void MtTilesetCentralWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    _animationTimer.stopTimer();

    MtTilesetResourceItem* item = qobject_cast<MtTilesetResourceItem*>(abstractItem);

    if (_tileset == item) {
        return;
    }

    if (_tileset) {
        _tileset->unloadPixmaps();
        _tileset->disconnect(this);
    }
    _tileset = item;

    _graphicsScene->clear();
    _graphicsItem = nullptr;

    if (_tileset) {
        _tileset->loadPixmaps();

        _graphicsItem = new MtTilesetGraphicsItem(_tileset);
        _graphicsScene->addItem(_graphicsItem);

        onMtTilesetDataChanged();
    }
    else {
        _ui->animationFrameLabel->clear();
    }

    setEnabled(item != nullptr);
}

void MtTilesetCentralWidget::updateFrameLabel()
{
    Q_ASSERT(_tileset);
    Q_ASSERT(_graphicsItem);

    const auto& pixmaps = _tileset->pixmaps();

    if (!pixmaps.empty()) {
        unsigned index = _graphicsItem->animationFrameIndex();
        _ui->animationFrameLabel->setText(tr("Frame %1").arg(index));
    }
    else {
        _ui->animationFrameLabel->clear();
    }
}

void MtTilesetCentralWidget::clearGui()
{
    _animationTimer.setEnabled(false);
    _ui->animationFrameLabel->clear();
}

void MtTilesetCentralWidget::onMtTilesetDataChanged()
{
    Q_ASSERT(_tileset);

    _animationTimer.setEnabled(_tileset->tilesetInput() != nullptr);

    if (const auto& ti = _tileset->tilesetInput()) {
        _animationTimer.setAnimationDelay(ti->animationFrames.animationDelay);
    }

    _graphicsItem->reloadAnimationFrame();

    // recenter graphics view
    _ui->graphicsView->viewport()->update();
    _graphicsScene->setSceneRect(_graphicsItem->boundingRect());
}

void MtTilesetCentralWidget::onAnimationFrameAdvance()
{
    _graphicsItem->nextAnimationFrame();
    updateFrameLabel();
}

void MtTilesetCentralWidget::onPreviousClicked()
{
    _animationTimer.stopTimer();
    _graphicsItem->prevAnimationFrame();
    updateFrameLabel();
}

void MtTilesetCentralWidget::onNextClicked()
{
    _animationTimer.stopTimer();
    _graphicsItem->nextAnimationFrame();
    updateFrameLabel();
}

MtTilesetGraphicsItem::MtTilesetGraphicsItem(MtTilesetResourceItem* item)
    : QGraphicsItem()
    , _tileset(item)
    , _animationFrameIndex(0)
{
    Q_ASSERT(_tileset);

    reloadAnimationFrame();
}

void MtTilesetGraphicsItem::setAnimationFrameIndex(int index)
{
    Q_ASSERT(_tileset);

    const auto& pixmaps = _tileset->pixmaps();

    if (!pixmaps.empty()) {
        if (index < 0) {
            index = pixmaps.size() - 1;
        }
        else if (index >= pixmaps.size()) {
            index = 0;
        }
        _animationFrameIndex = index;

        _pixmap = pixmaps.at(index);
    }
    else {
        _pixmap = QPixmap();
        _animationFrameIndex = 0;
    }

    update();
}

QRectF MtTilesetGraphicsItem::boundingRect() const
{
    unsigned w = _pixmap.width();
    unsigned h = _pixmap.height();

    return QRectF(0, 0, w, h);
}

void MtTilesetGraphicsItem::paint(QPainter* painter,
                                  const QStyleOptionGraphicsItem*, QWidget* widget)
{
    if (_pixmap.isNull()) {
        return;
    }

    painter->drawPixmap(0, 0, _pixmap);

    // draw grid

    painter->save();

    painter->setBrush(QBrush());
    painter->setPen(createCosmeticPen(GRID_COLOR, widget));

    const unsigned w = _pixmap.width();
    const unsigned h = _pixmap.height();

    painter->drawRect(0, 0, w, h);

    for (unsigned x = 16; x < w; x += 16) {
        painter->drawLine(x, 0, x, h);
    }
    for (unsigned y = 16; y < h; y += 16) {
        painter->drawLine(0, y, w, y);
    }

    painter->restore();
}
