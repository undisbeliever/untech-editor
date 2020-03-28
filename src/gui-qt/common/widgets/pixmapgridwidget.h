/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QPixmap>
#include <QVector>
#include <QWidget>

namespace UnTech {
namespace GuiQt {

class PixmapGridWidget : public QWidget {
    Q_OBJECT

public:
    PixmapGridWidget(QWidget* parent = nullptr);
    ~PixmapGridWidget() = default;

    const QColor& backgroundColor() const { return _backgroundColor; }
    void setBackgroundColor(const QColor& color);

    const QColor& gridColor() const { return _gridColor; }
    void setGridColor(const QColor& color);

    const QSize& cellSize() const { return _cellSize; }
    void setCellSize(const QSize& cellSize);

    const QVector<QPixmap>& pixmaps() const { return _pixmaps; }
    void setPixmaps(const QVector<QPixmap>& pixmaps);

    void setPixmap(int index, const QPixmap& pixmap);

    int selected() const { return _selected; }
    void setSelected(int selected);
    void clearSelected() { setSelected(-1); }

    virtual bool hasHeightForWidth() const override;
    virtual int heightForWidth(int width) const override;

    int columnCount() const;
    int indexAt(const QPoint& point) const;

protected:
    virtual void paintEvent(QPaintEvent* event) override;
    virtual void mousePressEvent(QMouseEvent* event) override;

signals:
    void cellClicked(int index);

private:
    void updateWindowSize();

private:
    QColor _backgroundColor;
    QColor _gridColor;
    QSize _cellSize;
    QVector<QPixmap> _pixmaps;
    int _selected;
};
}
}
