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

    _ui->region->addItem("NTSC", 1000000000 / 60);
    _ui->region->addItem("PAL", 1000000000 / 50);

    _ui->graphicsView->setScene(_graphicsScene);

    setEnabled(false);

    updateGui();

    connect(_ui->playButton, &QToolButton::toggled,
            this, &PaletteCentralWidget::onPlayButtonToggled);
}

PaletteCentralWidget::~PaletteCentralWidget() = default;

ResourceTypeIndex PaletteCentralWidget::resourceTypeIndex() const
{
    return ResourceTypeIndex::PALETTE;
}

void PaletteCentralWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    stopAnimation();

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

        _graphicsScene->setSceneRect(_graphicsItem->boundingRect());

        _ui->graphicsView->viewport()->update();
    }

    setEnabled(item != nullptr);

    updateGui();
}

void PaletteCentralWidget::updateGui()
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

void PaletteCentralWidget::clearGui()
{
    stopAnimation();
    _ui->animationFrameLabel->clear();
}

void PaletteCentralWidget::onPlayButtonToggled()
{
    if (_ui->playButton->isChecked()
        && _palette && _graphicsItem
        && _palette->paletteData()
        && _palette->paletteData()->nFrames() > 0) {

        _timer.start(1, Qt::PreciseTimer, this);
        _elapsed.restart();
        _nsSinceLastFrame = _elapsed.nsecsElapsed();
        _animationTicks = 0;

        _graphicsItem->setFrameIndex(0);
    }
    else {
        _ui->playButton->setChecked(false);

        _timer.stop();
        if (_graphicsItem) {
            _graphicsItem->setFrameIndex(-1);
        }
    }

    updateGui();

    _graphicsScene->setSceneRect(_graphicsItem->boundingRect());
}

void PaletteCentralWidget::stopAnimation()
{
    _ui->playButton->setChecked(false);
}

void PaletteCentralWidget::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == _timer.timerId()) {
        const auto& pal = _palette->paletteData();

        if (_graphicsItem && pal) {
            unsigned nsPerFrame = _ui->region->currentData().toUInt();
            qint64 nsecsElapsed = _elapsed.nsecsElapsed();
            qint64 ns = nsecsElapsed - _nsSinceLastFrame;

            constexpr unsigned TPS = ANIMATION_TICKS_PER_SECOND;
            unsigned ticksPerFrame = (nsPerFrame == 1000000000 / 60) ? TPS / 60 : TPS / 50;

            if (ns > nsPerFrame * 6) {
                // stop if execution takes too long
                stopAnimation();
            }
            while (ns >= nsPerFrame) {
                ns -= nsPerFrame;
                _nsSinceLastFrame += nsPerFrame;

                _animationTicks += ticksPerFrame;

                if (_animationTicks >= pal->animationDelay) {
                    _graphicsItem->nextAnimationFrame();
                    _animationTicks = 0;

                    if (pal->nFrames() <= 0) {
                        stopAnimation();
                    }
                }
            }
        }
        else {
            stopAnimation();
        }

        updateGui();
    }
    else {
        QWidget::timerEvent(event);
    }
}

PaletteGraphicsItem::PaletteGraphicsItem(PaletteResourceItem* item)
    : QGraphicsItem()
    , _palette(item)
    , _frameIndex(-1)
{
    Q_ASSERT(_palette != nullptr);

    updatePixmap();
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
    const auto& pal = _palette->paletteData();

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

    for (unsigned x = PALETTE_SCALE; x < w; x += PALETTE_SCALE) {
        painter->drawLine(x, 0, x, h);
    }
    for (unsigned y = PALETTE_SCALE; y < h; y += PALETTE_SCALE) {
        painter->drawLine(0, y, w, y);
    }

    // draw frame rows

    painter->setPen(QPen(FRAME_LINE_COLOR, FRAME_LINE_WIDTH));

    unsigned fh = PALETTE_SCALE * pal->rowsPerFrame;

    for (unsigned y = 0; y <= h; y += fh) {
        painter->drawLine(-FRAME_OVERHANG, y, w + FRAME_OVERHANG, y);
    }

    painter->restore();
}
