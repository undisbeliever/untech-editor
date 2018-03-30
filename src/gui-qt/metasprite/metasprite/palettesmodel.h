/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

private slots:
    void onPaletteChanged(unsigned index);
    void onPaletteAdded(unsigned index);
    void onPaletteAboutToBeRemoved(unsigned index);
    void onPaletteMoved(unsigned fromIndex, unsigned toIndex);

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
