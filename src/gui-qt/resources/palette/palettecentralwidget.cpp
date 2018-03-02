/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettecentralwidget.h"
#include "paletteresourceitem.h"
#include "gui-qt/resources/palette/palettecentralwidget.ui.h"

using namespace UnTech::GuiQt::Resources;

const QColor PaletteGraphicsItem::LINE_COLOR = QColor(200, 200, 255, 128);
const QColor PaletteGraphicsItem::FRAME_LINE_COLOR = QColor(200, 100, 200, 192);

PaletteCentralWidget::PaletteCentralWidget(QWidget* parent)
    : AbstractResourceWidget(parent)
    , _ui(new Ui::PaletteCentralWidget)
    , _graphicsScene(new QGraphicsScene(this))
    , _palette(nullptr)
    , _graphicsItem(nullptr)
{
    _ui->setupUi(this);

    _animationTimer.setRegionCombo(_ui->region);
    _animationTimer.setPlayButton(_ui->playButton);

    _ui->graphicsView->setScene(_graphicsScene);

    setEnabled(false);

    updateFrameLabel();

    connect(&_animationTimer, &AnimationTimer::animationStarted,
            this, &PaletteCentralWidget::onAnimationStarted);
    connect(&_animationTimer, &AnimationTimer::animationFrameAdvance,
            this, &PaletteCentralWidget::onAnimationFrameAdvance);
    connect(&_animationTimer, &AnimationTimer::animationStopped,
            this, &PaletteCentralWidget::onAnimationStopped);
}

PaletteCentralWidget::~PaletteCentralWidget() = default;

ResourceTypeIndex PaletteCentralWidget::resourceTypeIndex() const
{
    return ResourceTypeIndex::PALETTE;
}

void PaletteCentralWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    _animationTimer.stopTimer();

    PaletteResourceItem* item = qobject_cast<PaletteResourceItem*>(abstractItem);

    if (_palette == item) {
        return;
    }

    if (_palette) {
        _palette->disconnect(this);
    }
    _palette = item;

    _graphicsScene->clear();
    _graphicsItem = nullptr;

    if (_palette) {
        _graphicsItem = new PaletteGraphicsItem(item);
        _graphicsScene->addItem(_graphicsItem);

        onPaletteDataChanged();
        updateFrameLabel();

        connect(_palette, &PaletteResourceItem::dataChanged,
                this, &PaletteCentralWidget::onPaletteDataChanged);
    }
    else {
        clearGui();
    }

    setEnabled(item != nullptr);
}

void PaletteCentralWidget::updateFrameLabel()
{
    if (_palette == nullptr) {
        clearGui();
        return;
    }

    unsigned nFrames = _palette->paletteData()->nFrames();
    int fIndex = _graphicsItem->frameIndex();

    if (fIndex >= 0 && nFrames > 0) {
        _ui->animationFrameLabel->setText(
            tr("Frame %1").arg(fIndex % nFrames));
    }
    else {
        _ui->animationFrameLabel->clear();
    }
}

void PaletteCentralWidget::centerGraphicsItem()
{
    Q_ASSERT(_graphicsItem);

    _graphicsScene->setSceneRect(_graphicsItem->boundingRect());
    _ui->graphicsView->viewport()->update();
}

void PaletteCentralWidget::clearGui()
{
    _animationTimer.setEnabled(false);
    _ui->animationFrameLabel->clear();
}

void PaletteCentralWidget::onPaletteDataChanged()
{
    Q_ASSERT(_palette);
    const auto& pData = _palette->paletteData();
    Q_ASSERT(pData);

    _animationTimer.setAnimationDelay(pData->animationDelay);
    _animationTimer.setEnabled(pData->nFrames() > 0);

    centerGraphicsItem();
}

void PaletteCentralWidget::onAnimationStarted()
{
    if (_graphicsItem) {
        _graphicsItem->setFrameIndex(0);
        centerGraphicsItem();
        updateFrameLabel();
    }
}

void PaletteCentralWidget::onAnimationFrameAdvance()
{
    _graphicsItem->nextAnimationFrame();
    updateFrameLabel();
}

void PaletteCentralWidget::onAnimationStopped()
{
    if (_graphicsItem) {
        _graphicsItem->setFrameIndex(-1);
        centerGraphicsItem();
        updateFrameLabel();
    }
}

PaletteGraphicsItem::PaletteGraphicsItem(PaletteResourceItem* item)
    : QGraphicsObject()
    , _palette(item)
    , _frameIndex(-1)
{
    Q_ASSERT(_palette != nullptr);

    updatePixmap();

    connect(_palette, &PaletteResourceItem::imageFilenameChanged,
            this, &PaletteGraphicsItem::updatePixmap);
}

void PaletteGraphicsItem::updatePixmap()
{
    prepareGeometryChange();

    const std::string& fn = _palette->paletteData()->paletteImageFilename;

    if (!fn.empty()) {
        _pixmap.load(QString::fromStdString(fn));
    }
    else {
        _pixmap = QPixmap();
    }
}

void PaletteGraphicsItem::setFrameIndex(int index)
{
    if (_frameIndex != index) {
        update();
    }

    if ((_frameIndex < 0 && index >= 0)
        || (_frameIndex >= 0 && index < 0)) {

        prepareGeometryChange();
    }

    _frameIndex = index;
}

QRectF PaletteGraphicsItem::boundingRect() const
{
    const auto& pal = _palette->paletteData();

    unsigned w = _pixmap.width();
    unsigned h = _pixmap.height();

    if (_frameIndex >= 0 && pal && pal->nFrames() > 0) {
        h = pal->rowsPerFrame;
    }

    return QRectF(-FRAME_OVERHANG, -FRAME_LINE_WIDTH,
                  w * PALETTE_SCALE + FRAME_OVERHANG * 2, h * PALETTE_SCALE + FRAME_LINE_WIDTH * 2);
}

void PaletteGraphicsItem::paint(QPainter* painter,
                                const QStyleOptionGraphicsItem*, QWidget*)
{
    const RES::PaletteInput* pal = _palette->paletteData();

    if (_pixmap.isNull() || pal == nullptr) {
        return;
    }

    // draw image

    unsigned w = _pixmap.width();
    unsigned h = _pixmap.height();
    unsigned sx = 0;
    unsigned sy = 0;

    if (_frameIndex >= 0 && pal->nFrames() > 0) {
        h = pal->rowsPerFrame;
        sy = (_frameIndex % pal->nFrames() + pal->skipFirstFrame) * pal->rowsPerFrame;
    }

    unsigned sw = w;
    unsigned sh = h;
    w *= PALETTE_SCALE;
    h *= PALETTE_SCALE;

    painter->drawPixmap(0, 0, w, h, _pixmap, sx, sy, sw, sh);

    // draw grid

    painter->save();

    painter->setBrush(QBrush());
    painter->setPen(QPen(LINE_COLOR, LINE_WIDTH));

    painter->drawRect(-LINE_WIDTH, -LINE_WIDTH, w + LINE_WIDTH, h + LINE_WIDTH);

    Q_ASSERT(PALETTE_SCALE > 0);
    for (unsigned x = PALETTE_SCALE; x < w; x += PALETTE_SCALE) {
        painter->drawLine(x, 0, x, h);
    }
    for (unsigned y = PALETTE_SCALE; y < h; y += PALETTE_SCALE) {
        painter->drawLine(0, y, w, y);
    }

    // draw frame rows

    painter->setPen(QPen(FRAME_LINE_COLOR, FRAME_LINE_WIDTH));

    unsigned fh = PALETTE_SCALE * pal->rowsPerFrame;
    if (fh > 0) {
        for (unsigned y = 0; y <= h; y += fh) {
            painter->drawLine(-FRAME_OVERHANG, y, w + FRAME_OVERHANG, y);
        }
    }

    painter->restore();
}
