/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabbselectionoutlineitem.h"
#include "models/common/aabb.h"
#include "models/common/ms8aabb.h"
#include <QGraphicsItem>
#include <memory>

namespace UnTech {
namespace GuiQt {

/*
 * The PixmapGraphicsItem class is a GraphicsItem that displays a
 * flippable QPixmap whose:
 *   * Position is integer aligned.
 *   * Position is bound to a range.
 *   * Selection outline is inserted in the top-level of the scene with a
 *     large zValue so it is drawn on top of all the other items.
 *
 * NOTE:
 * The transformation of this item MUST NOT change when it is selected.
 */
class PixmapGraphicsItem : public QGraphicsItem {
public:
    explicit PixmapGraphicsItem(QGraphicsItem* parent = nullptr);
    ~PixmapGraphicsItem() = default;

    const QPixmap& pixmap() const { return _pixmap; }
    void setPixmap(const QPixmap& pixmap);

    const QRect& range() const { return _range; }
    void setRange(const QRect& range);
    inline void setRange(int x, int y, int width, int height)
    {
        setRange(QRect(x, y, width, height));
    }

    inline upoint posUpoint() const
    {
        const QPointF& p = pos();
        return upoint(p.x(), p.y());
    }
    inline ms8point posMs8point() const
    {
        const QPointF& p = pos();
        return ms8point(p.x(), p.y());
    }

    inline void setPos(const QPoint& p)
    {
        QGraphicsItem::setPos(p.x(), p.y());
    }
    inline void setPos(int x, int y)
    {
        QGraphicsItem::setPos(x, y);
    }
    inline void setPos(const upoint& p)
    {
        QGraphicsItem::setPos(p.x, p.y);
    }
    inline void setPos(const ms8point& p)
    {
        QGraphicsItem::setPos(p.x, p.y);
    }

    bool hFlip() const { return _hFlip; }
    bool vFlip() const { return _vFlip; }
    void setFlip(bool hFlip, bool vFlip);

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    void updateSelectionOutline();
    QPoint validatePosition(const QPointF& pos) const;

private:
    std::unique_ptr<AabbSelectionOutlineItem> _selectionOutline;

    QPixmap _pixmap;
    QRect _range;

    bool _hFlip;
    bool _vFlip;
};
}
}
