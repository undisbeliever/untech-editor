/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "document.h"
#include "models/metasprite/metasprite.h"
#include <QAbstractListModel>
#include <QList>
#include <QPixmap>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;
class AddRemovePalette;
class RaisePalette;
class LowerPalette;
class ChangePaletteColor;

class PalettesModel : public QAbstractListModel {
    Q_OBJECT

public:
    constexpr static unsigned SCALE = 15;

public:
    explicit PalettesModel(QObject* parent = nullptr);
    ~PalettesModel() = default;

    void setDocument(Document* document);

    QModelIndex toModelIndex(unsigned index) const;

    virtual int rowCount(const QModelIndex& parent) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant data(const QModelIndex& index, int role) const final;

protected:
    friend class AddRemovePalette;
    void insertPalette(unsigned index, const Snes::Palette4bpp& pal);
    void removePalette(unsigned index);

    friend class RaisePalette;
    void raisePalette(unsigned index);

    friend class LowerPalette;
    void lowerPalette(unsigned index);

    friend class ChangePaletteColor;
    void setPaletteColor(unsigned paletteIndex, unsigned colorIndex,
                         const Snes::SnesColor& color);

private:
    void updateAllPixmaps();
    QPixmap palettePixmap(unsigned index);

private:
    Document* _document;

    QList<QPixmap> _palettePixmaps;
};
}
}
}
}
