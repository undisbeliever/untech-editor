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
const QColor MtTilesetGraphicsItem::ERROR_COLOR = QColor(255, 0, 50, 128);

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
        _tileset->disconnect(this);
    }
    _tileset = item;

    _graphicsScene->clear();
    _graphicsItem = nullptr;

    if (_tileset) {
        _graphicsItem = new MtTilesetGraphicsItem(_tileset);
        _graphicsScene->addItem(_graphicsItem);

        onMtTilesetDataChanged();

        connect(_tileset, &MtTilesetResourceItem::dataChanged,
                this, &MtTilesetCentralWidget::onMtTilesetDataChanged);
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

    if (_graphicsItem->animationFrameIndex() >= 0) {
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
    : QGraphicsObject()
    , _tileset(item)
    , _commonErrors(nullptr)
    , _animationFrameIndex(0)
{
    Q_ASSERT(_tileset);

    loadPixmaps();
    reloadAnimationFrame();

    updateInvalidTiles();

    connect(_tileset, &MtTilesetResourceItem::frameImageFilenamesChanged,
            this, &MtTilesetGraphicsItem::loadPixmaps);

    connect(_tileset, &AbstractResourceItem::errorListChanged,
            this, &MtTilesetGraphicsItem::updateInvalidTiles);
}

void MtTilesetGraphicsItem::loadPixmaps()
{
    Q_ASSERT(_tileset);
    const auto& _tilesetInput = _tileset->tilesetInput();

    _pixmaps.clear();

    if (_tilesetInput) {
        const auto& filenames = _tilesetInput->animationFrames.frameImageFilenames;
        _pixmaps.reserve(filenames.size());
        for (const auto& fn : filenames) {
            _pixmaps.append(QPixmap(QString::fromStdString(fn), "PNG"));
        }

        if (_animationFrameIndex > _pixmaps.size()) {
            setAnimationFrameIndex(0);
        }
    }

    prepareGeometryChange();
    update();
}

void MtTilesetGraphicsItem::setAnimationFrameIndex(int index)
{
    Q_ASSERT(_tileset);

    if (!_pixmaps.empty()) {
        if (index < 0) {
            index = _pixmaps.size() - 1;
        }
        else if (index >= _pixmaps.size()) {
            index = 0;
        }
        _animationFrameIndex = index;
    }
    else {
        _animationFrameIndex = -1;
    }

    for (int i = 0; i < _frameErrors.size(); i++) {
        _frameErrors.at(i)->setVisible(i == index);
    }

    update();
}

QRectF MtTilesetGraphicsItem::boundingRect() const
{
    if (_animationFrameIndex < 0 || _animationFrameIndex > _pixmaps.size()) {
        return QRectF();
    }
    const QPixmap& pixmap = _pixmaps.at(_animationFrameIndex);

    unsigned w = pixmap.width();
    unsigned h = pixmap.height();

    return QRectF(0, 0, w, h);
}

void MtTilesetGraphicsItem::paint(QPainter* painter,
                                  const QStyleOptionGraphicsItem*, QWidget* widget)
{
    if (_animationFrameIndex < 0 || _animationFrameIndex > _pixmaps.size()) {
        return;
    }
    const QPixmap& pixmap = _pixmaps.at(_animationFrameIndex);

    painter->drawPixmap(0, 0, pixmap);

    // draw grid

    painter->save();

    painter->setBrush(QBrush());
    painter->setPen(createCosmeticPen(GRID_COLOR, widget));

    const unsigned w = pixmap.width();
    const unsigned h = pixmap.height();

    painter->drawRect(0, 0, w, h);

    for (unsigned x = 16; x < w; x += 16) {
        painter->drawLine(x, 0, x, h);
    }
    for (unsigned y = 16; y < h; y += 16) {
        painter->drawLine(0, y, w, y);
    }

    painter->restore();
}

void MtTilesetGraphicsItem::updateInvalidTiles()
{
    // delete all the old error items
    {
        if (_commonErrors) {
            delete _commonErrors;
        }
        _commonErrors = nullptr;

        qDeleteAll(_frameErrors);
        _frameErrors.clear();
    }

    // Create new parent containers for the errors
    {
        QSize s;
        for (auto& p : _pixmaps) {
            s = s.expandedTo(p.size());
        }

        QRectF r(0, 0, s.width(), s.height());
        unsigned nFrames = _pixmaps.size();
        for (unsigned i = 0; i < nFrames; i++) {
            auto* item = new QGraphicsRectItem(r, this);
            item->setBrush(Qt::NoBrush);
            item->setPen(Qt::NoPen);
            item->hide();
            _frameErrors.append(item);
        }

        QGraphicsRectItem* ce = new QGraphicsRectItem(r, this);
        ce->setBrush(Qt::NoBrush);
        ce->setPen(Qt::NoPen);
        _commonErrors = ce;
    }

    // create the error items
    {
        Q_ASSERT(QGuiApplication::topLevelWindows().size() > 0);
        QWindow* widget = QGuiApplication::topLevelWindows().first();

        QBrush errorBrush(ERROR_COLOR);
        QPen errorPen = createCosmeticPen(ERROR_COLOR, widget);

        const auto& invalidTiles = _tileset->errorList().invalidImageTiles();

        for (const auto& tile : invalidTiles) {
            QRectF r(tile.x, tile.y, tile.size, tile.size);

            QGraphicsItem* parent = _commonErrors;
            if (tile.showFrameId()) {
                parent = _frameErrors.at(tile.frameId);
            }

            QGraphicsRectItem* tileItem = new QGraphicsRectItem(r, parent);
            tileItem->setBrush(errorBrush);
            tileItem->setPen(errorPen);
            tileItem->setToolTip(QString::fromLatin1("(%1, %2) %3").arg(tile.x).arg(tile.y).arg(toolTipForType(tile.reason)));
        }
    }

    // show errors for the current frame
    if (_animationFrameIndex >= 0 && _animationFrameIndex < _frameErrors.size()) {
        _frameErrors.at(_animationFrameIndex)->show();
    }
}

const QString& MtTilesetGraphicsItem::toolTipForType(const RES::ErrorList::InvalidTileReason& reason)
{
    using ITR = RES::ErrorList::InvalidTileReason;

    static const QString NULL_STRING;
    static const QString NO_PALETTE_FOUND = tr("No palette found");
    static const QString NOT_SAME_PALETTE = tr("Must use the same palette in each frame");
    static const QString TOO_MANY_COLORS = tr("Too many colors");

    switch (reason) {
    case ITR::NO_PALETTE_FOUND:
        return NO_PALETTE_FOUND;

    case ITR::NOT_SAME_PALETTE:
        return NOT_SAME_PALETTE;

    case ITR::TOO_MANY_COLORS:
        return TOO_MANY_COLORS;
    };

    return NULL_STRING;
}