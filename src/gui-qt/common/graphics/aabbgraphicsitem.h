/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "aabbselectionoutlineitem.h"
#include "models/common/aabb.h"
#include "models/common/ms8aabb.h"
#include <QBrush>
#include <QGraphicsItem>
#include <QPen>
#include <memory>

namespace UnTech {
namespace GuiQt {

/*
 * The AabbGraphicsItem class is a rectangular GraphicsItem whose
 *   * Position and size is integer aligned.
 *   * Position and size is bound to a range.
 *   * Selection outline is inserted in the top-level of the scene with a
 *     large zValue so it is drawn on top of all the other items.
 *
 * NOTE:
 * The transformation of this item MUST NOT change when it is selected.
 */
class AabbGraphicsItem : public QGraphicsItem {
public:
    explicit AabbGraphicsItem(QGraphicsItem* parent = nullptr);
    ~AabbGraphicsItem() = default;

    const QRect& range() const { return _range; }
    void setRange(const QRect& range);
    inline void setRange(int x, int y, const usize& s)
    {
        setRange(QRect(x, y, s.width, s.height));
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

    QSize size() const { return _size; }
    inline usize sizeUsize() const
    {
        return usize(_size.width(), _size.height());
    }

    void setSize(int width, int height);
    inline void setSize(const QSize& s)
    {
        setSize(s.width(), s.height());
    }
    inline void setSize(const usize& s)
    {
        setSize(s.width, s.height);
    }
    inline void setSize(unsigned squareSize)
    {
        setSize(squareSize, squareSize);
    }

    inline QRect rect() const
    {
        const QPointF& p = pos();
        return QRect(p.x(), p.y(), _size.width(), _size.height());
    }
    inline urect rectUrect() const
    {
        const QPointF& p = pos();
        return urect(p.x(), p.y(), _size.width(), _size.height());
    }
    inline ms8rect rectMs8rect() const
    {
        const QPointF& p = pos();
        return ms8rect(p.x(), p.y(), _size.width(), _size.height());
    }

    void setRect(const QRect& rect);
    void setRect(const urect& rect);
    void setRect(const ms8rect& rect);
    void setRect(const upoint& point, unsigned squareSize);
    void setRect(const ms8point& point, unsigned squareSize);

    const QPen& pen() const { return _pen; }
    void setPen(const QPen& pen);

    const QBrush& brush() const { return _brush; }
    void setBrush(const QBrush& brush);

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) override;

protected:
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
    void updateSelectionOutline();

private:
    QSize _size;
    QRect _range;

    QPen _pen;
    QBrush _brush;

    std::unique_ptr<AabbSelectionOutlineItem> _selectionOutline;
};
}
}
