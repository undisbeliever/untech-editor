/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettesmodel.h"
#include "accessors.h"
#include "resourceitem.h"

#include <QImage>
#include <QPixmap>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

PalettesModel::PalettesModel(QObject* parent)
    : QAbstractListModel(parent)
    , _resourceItem(nullptr)
{
}

void PalettesModel::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    beginResetModel();

    if (_resourceItem) {
        _resourceItem->disconnect(this);
        _resourceItem->paletteList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    updateAllPixmaps();

    if (_resourceItem) {
        connect(_resourceItem->paletteList(), &PaletteList::dataChanged,
                this, &PalettesModel::onPaletteChanged);
        connect(_resourceItem->paletteList(), &PaletteList::itemAdded,
                this, &PalettesModel::onPaletteAdded);
        connect(_resourceItem->paletteList(), &PaletteList::itemAboutToBeRemoved,
                this, &PalettesModel::onPaletteAboutToBeRemoved);
        connect(_resourceItem->paletteList(), &PaletteList::itemMoved,
                this, &PalettesModel::onPaletteMoved);
    }

    endResetModel();
}

void PalettesModel::updateAllPixmaps()
{
    _palettePixmaps.clear();

    if (_resourceItem) {
        unsigned nPalettes = _resourceItem->frameSet()->palettes.size();
        for (unsigned i = 0; i < nPalettes; i++) {
            _palettePixmaps.append(palettePixmap(i));
        }
    }
}

QPixmap PalettesModel::palettePixmap(unsigned index)
{
    const auto& palette = _resourceItem->frameSet()->palettes.at(index);

    QImage img(palette.N_COLORS, 1, QImage::Format_RGB32);
    QRgb* imgBits = reinterpret_cast<QRgb*>(img.bits());

    for (const Snes::SnesColor& sc : palette.colors()) {
        const auto& rgb = sc.rgb();

        *imgBits++ = qRgb(rgb.red, rgb.green, rgb.blue);
    }

    return QPixmap::fromImage(img).scaled(palette.N_COLORS * SCALE, SCALE);
}

QModelIndex PalettesModel::toModelIndex(unsigned index) const
{
    if (_resourceItem == nullptr
        || _resourceItem->frameSet() == nullptr
        || index >= _resourceItem->frameSet()->palettes.size()) {

        return QModelIndex();
    }

    return createIndex(index, 0);
}

int PalettesModel::rowCount(const QModelIndex& parent) const
{
    if (_resourceItem == nullptr || parent.isValid()) {
        return 0;
    }
    return _resourceItem->frameSet()->palettes.size();
}

Qt::ItemFlags PalettesModel::flags(const QModelIndex& index) const
{
    if (_resourceItem == nullptr
        || index.row() < 0
        || unsigned(index.row()) >= _resourceItem->frameSet()->palettes.size()) {
        return 0;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PalettesModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DecorationRole
        || _resourceItem == nullptr
        || index.row() < 0
        || unsigned(index.row()) >= _resourceItem->frameSet()->palettes.size()
        || index.column() != 0) {
        return QVariant();
    }

    return _palettePixmaps.at(index.row());
}

void PalettesModel::onPaletteChanged(size_t index)
{
    Q_ASSERT(_resourceItem != nullptr);

    auto& palettes = _resourceItem->frameSet()->palettes;
    Q_ASSERT(index < palettes.size());

    _palettePixmaps.replace(index, palettePixmap(index));

    emit dataChanged(createIndex(index, 0),
                     createIndex(index, 1),
                     { Qt::DecorationRole });
}

void PalettesModel::onPaletteAdded(size_t index)
{
    Q_ASSERT(_resourceItem != nullptr);

    auto& palettes = _resourceItem->frameSet()->palettes;
    Q_ASSERT(index <= palettes.size());

    layoutAboutToBeChanged();

    _palettePixmaps.insert(index, palettePixmap(index));

    layoutChanged();
}

void PalettesModel::onPaletteAboutToBeRemoved(size_t index)
{
    Q_ASSERT(_resourceItem != nullptr);

    auto& palettes = _resourceItem->frameSet()->palettes;
    Q_ASSERT(index < palettes.size());

    layoutAboutToBeChanged();

    _palettePixmaps.removeAt(index);

    layoutChanged();
}

void PalettesModel::onPaletteMoved(size_t fromIndex, size_t toIndex)
{
    Q_ASSERT(_resourceItem != nullptr);

    auto& palettes = _resourceItem->frameSet()->palettes;
    Q_ASSERT((int)palettes.size() == _palettePixmaps.size());
    Q_ASSERT(fromIndex < palettes.size());
    Q_ASSERT(toIndex < palettes.size());

    int toSignal = toIndex;
    if (toIndex >= fromIndex) {
        toSignal++;
    }

    layoutAboutToBeChanged();

    _palettePixmaps.swapItemsAt(fromIndex, toIndex);

    layoutChanged();
}
