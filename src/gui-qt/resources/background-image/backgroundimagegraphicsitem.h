/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QGraphicsObject>
#include <QPixmap>

namespace UnTech {
namespace Resources {
struct PaletteData;
struct BackgroundImageData;
}
}

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace BackgroundImage {

namespace RES = UnTech::Resources;

class BackgroundImageGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit BackgroundImageGraphicsItem(QGraphicsItem* parent = nullptr);
    ~BackgroundImageGraphicsItem() = default;

    unsigned paletteFrame() const { return _paletteFrame; }
    bool hasTransparentTiles() const { return _transparentTiles; }

    void setTransparentTiles(bool transparentTiles) { _transparentTiles = transparentTiles; }

    bool hasPixmaps() { return !_pixmaps.isEmpty(); }

    void drawPixmaps(const RES::BackgroundImageData&, const RES::PaletteData&);

public slots:
    void removePixmaps();

    void resetPaletteFrame();
    void nextPaletteFrame();
    void setPaletteFrame(unsigned paletteFrame);

signals:
    void paletteFrameChanged();

public:
    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

private:
    QRect _boundingRect;
    QList<QPixmap> _pixmaps;

    unsigned _paletteFrame;
    bool _transparentTiles;
};

}
}
}
}
