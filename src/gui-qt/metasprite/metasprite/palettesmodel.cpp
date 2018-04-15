/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettesmodel.h"
#include "accessors.h"
#include "document.h"

#include <QImage>
#include <QPixmap>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

PalettesModel::PalettesModel(QObject* parent)
    : QAbstractListModel(parent)
    , _document(nullptr)
{
}

void PalettesModel::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    beginResetModel();

    if (_document) {
        _document->disconnect(this);
        _document->paletteList()->disconnect(this);
    }
    _document = document;

    updateAllPixmaps();

    if (_document) {
        connect(_document->paletteList(), &PaletteList::dataChanged,
                this, &PalettesModel::onPaletteChanged);
        connect(_document->paletteList(), &PaletteList::itemAdded,
                this, &PalettesModel::onPaletteAdded);
        connect(_document->paletteList(), &PaletteList::itemAboutToBeRemoved,
                this, &PalettesModel::onPaletteAboutToBeRemoved);
        connect(_document->paletteList(), &PaletteList::itemMoved,
                this, &PalettesModel::onPaletteMoved);
    }

    endResetModel();
}

void PalettesModel::updateAllPixmaps()
{
    _palettePixmaps.clear();

    if (_document) {
        unsigned nPalettes = _document->frameSet()->palettes.size();
        for (unsigned i = 0; i < nPalettes; i++) {
            _palettePixmaps.append(palettePixmap(i));
        }
    }
}

QPixmap PalettesModel::palettePixmap(unsigned index)
{
    const auto& palette = _document->frameSet()->palettes.at(index);

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
    return createIndex(index, 0);
}

int PalettesModel::rowCount(const QModelIndex& parent) const
{
    if (_document == nullptr || parent.isValid()) {
        return 0;
    }
    return _document->frameSet()->palettes.size();
}

Qt::ItemFlags PalettesModel::flags(const QModelIndex& index) const
{
    if (_document == nullptr
        || index.row() < 0
        || unsigned(index.row()) >= _document->frameSet()->palettes.size()) {
        return 0;
    }
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant PalettesModel::data(const QModelIndex& index, int role) const
{
    if (role != Qt::DecorationRole
        || _document == nullptr
        || index.row() < 0
        || unsigned(index.row()) >= _document->frameSet()->palettes.size()
        || index.column() != 0) {
        return QVariant();
    }

    return _palettePixmaps.at(index.row());
}

void PalettesModel::onPaletteChanged(size_t index)
{
    Q_ASSERT(_document != nullptr);

    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(index < palettes.size());

    _palettePixmaps.replace(index, palettePixmap(index));

    emit dataChanged(createIndex(index, 0),
                     createIndex(index, 1),
                     { Qt::DecorationRole });
}

void PalettesModel::onPaletteAdded(size_t index)
{
    Q_ASSERT(_document != nullptr);

    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(index <= palettes.size());

    beginInsertRows(QModelIndex(), index, index);

    _palettePixmaps.insert(index, palettePixmap(index));

    endInsertRows();
}

void PalettesModel::onPaletteAboutToBeRemoved(size_t index)
{
    Q_ASSERT(_document != nullptr);

    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(index < palettes.size());

    beginRemoveRows(QModelIndex(), index, index);

    _palettePixmaps.removeAt(index);

    endRemoveRows();
}

void PalettesModel::onPaletteMoved(size_t fromIndex, size_t toIndex)
{
    Q_ASSERT(_document != nullptr);

    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT((int)palettes.size() == _palettePixmaps.size());
    Q_ASSERT(fromIndex < palettes.size());
    Q_ASSERT(toIndex < palettes.size());

    int toSignal = toIndex;
    if (toIndex >= fromIndex) {
        toSignal++;
    }

    layoutAboutToBeChanged();

    _palettePixmaps.swap(fromIndex, toIndex);

    layoutChanged();
}
