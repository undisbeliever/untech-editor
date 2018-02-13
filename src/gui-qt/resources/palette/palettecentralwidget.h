/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../abstractresourcewidget.h"
#include <QBasicTimer>
#include <QElapsedTimer>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {

namespace Ui {
class PaletteCentralWidget;
}

class Document;
class AbstractResourceItem;
class PaletteResourceItem;
class PaletteGraphicsItem;

class PaletteCentralWidget : public AbstractResourceWidget {
    Q_OBJECT

    static constexpr unsigned ANIMATION_TICKS_PER_SECOND = 300;

public:
    explicit PaletteCentralWidget(QWidget* parent = nullptr);
    ~PaletteCentralWidget();

    virtual ResourceTypeIndex resourceTypeIndex() const final;

    virtual void setResourceItem(AbstractResourceItem* item) final;

    virtual void timerEvent(QTimerEvent* event) final;

private slots:
    void clearGui();
    void updateGui();

    void onPlayButtonToggled();
    void stopAnimation();

private:
    std::unique_ptr<Ui::PaletteCentralWidget> _ui;
    QGraphicsScene* _graphicsScene;

    PaletteResourceItem* _palette;

    PaletteGraphicsItem* _graphicsItem;

    QBasicTimer _timer;
    QElapsedTimer _elapsed;
    qint64 _nsSinceLastFrame;
    unsigned _animationTicks;
};

class PaletteGraphicsItem : public QGraphicsItem {
public:
    static constexpr int PALETTE_SCALE = 24;
    static constexpr int LINE_WIDTH = 1;
    static constexpr int FRAME_LINE_WIDTH = 2;
    static constexpr int FRAME_OVERHANG = PALETTE_SCALE / 2;

    static const QColor LINE_COLOR;
    static const QColor FRAME_LINE_COLOR;

public:
    explicit PaletteGraphicsItem(PaletteResourceItem* item);
    ~PaletteGraphicsItem() = default;

    void updatePixmap();

    void setFrameIndex(int index);
    void nextAnimationFrame() { setFrameIndex(_frameIndex + 1); }
    int frameIndex() const { return _frameIndex; }

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

private:
    PaletteResourceItem* const _palette;
    QPixmap _pixmap;

    // if < 0 then the whole image is shown
    int _frameIndex;
};
}
}
}
