/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "resourceitem.h"
#include "models/metasprite/metasprite.h"
#include <QAbstractListModel>
#include <QList>
#include <QPixmap>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class ResourceItem;

class PalettesModel : public QAbstractListModel {
    Q_OBJECT

public:
    constexpr static unsigned SCALE = 15;

public:
    explicit PalettesModel(QObject* parent = nullptr);
    ~PalettesModel() = default;

    void setResourceItem(ResourceItem* resourceItem);

    QModelIndex toModelIndex(unsigned index) const;

    virtual int rowCount(const QModelIndex& parent) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant data(const QModelIndex& index, int role) const final;

private slots:
    void onPaletteChanged(size_t index);
    void onPaletteAdded(size_t index);
    void onPaletteAboutToBeRemoved(size_t index);
    void onPaletteMoved(size_t fromIndex, size_t toIndex);

private:
    void updateAllPixmaps();
    QPixmap palettePixmap(unsigned index);

private:
    ResourceItem* _resourceItem;

    QList<QPixmap> _palettePixmaps;
};
}
}
}
}
