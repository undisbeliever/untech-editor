/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/widgets/drawingpixmapgridwidget.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;
class TilesetPixmaps;

class AbstractTilesetWidget : public DrawingPixmapGridWidget {
    Q_OBJECT

public:
    AbstractTilesetWidget(QWidget* parent = nullptr);
    ~AbstractTilesetWidget() = default;

    void setDocument(Document* document);

protected slots:
    void onPaletteChanged(unsigned index);
    void updateBackgroundColor();
    void onSelectedColorChanged();

protected:
    Document* _document;
};

class SmallTilesetWidget : public AbstractTilesetWidget {
    Q_OBJECT

public:
    SmallTilesetWidget(QWidget* parent = nullptr);
    ~SmallTilesetWidget() = default;

    void setTilesetPixmaps(TilesetPixmaps* tilesetPixmaps);

protected:
    virtual void drawPixel(int tileId, const QPoint& point, bool first) final;

private slots:
    void onTilesetPixmapRedrawn();
    void onTilesetPixmapTileChanged(int tileId);

private:
    TilesetPixmaps* _tilesetPixmaps;
};

class LargeTilesetWidget : public AbstractTilesetWidget {
    Q_OBJECT

public:
    LargeTilesetWidget(QWidget* parent = nullptr);
    ~LargeTilesetWidget() = default;

    void setTilesetPixmaps(TilesetPixmaps* tilesetPixmaps);

protected:
    virtual void drawPixel(int tileId, const QPoint& point, bool first) final;

private slots:
    void onTilesetPixmapRedrawn();
    void onTilesetPixmapTileChanged(int tileId);

private:
    TilesetPixmaps* _tilesetPixmaps;
};
}
}
}
}
