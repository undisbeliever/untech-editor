/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "document.h"
#include "models/metasprite/metasprite.h"
#include <QPixmap>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;

class TilesetPixmaps : public QObject {
    Q_OBJECT

public:
    explicit TilesetPixmaps(QObject* parent = nullptr);
    ~TilesetPixmaps() = default;

    const Document* document() const { return _document; }
    void setDocument(Document* document);

    const QVector<QPixmap>& smallTileset() const { return _smallTileset; }
    const QVector<QPixmap>& largeTileset() const { return _largeTileset; }

    const QPixmap& smallTile(unsigned index) const;
    const QPixmap& largeTile(unsigned index) const;

protected:
    void setSmallTile(unsigned tileId, const Snes::Tile8px& tile);
    void setLargeTile(unsigned tileId, const Snes::Tile16px& tile);

signals:
    void pixmapsChanged();

    void pixmapsRedrawn();
    void smallTileChanged(int tileId);
    void largeTileChanged(int tileId);

private slots:
    void redrawTilesets();
    void onPaletteChanged(unsigned index);

    void onSmallTileChanged(unsigned tileId);
    void onLargeTileChanged(unsigned tileId);

private:
    const Snes::Palette4bpp& palette() const;

private:
    Document* _document;

    QVector<QPixmap> _smallTileset;
    QVector<QPixmap> _largeTileset;

    const QPixmap _blankSmallTile;
    const QPixmap _blankLargeTile;
};
}
}
}
}
