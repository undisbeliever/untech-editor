/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "palettesmodel.h"
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
    Q_ASSERT(document != nullptr);

    beginResetModel();

    _document = document;
    updateAllPixmaps();

    endResetModel();
}

void PalettesModel::updateAllPixmaps()
{
    _palettePixmaps.clear();

    unsigned nPalettes = _document->frameSet()->palettes.size();
    for (unsigned i = 0; i < nPalettes; i++) {
        _palettePixmaps.append(palettePixmap(i));
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

void PalettesModel::insertPalette(unsigned index, const Snes::Palette4bpp& pal)
{
    Q_ASSERT(_document != nullptr);

    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(index <= palettes.size());

    beginInsertRows(QModelIndex(), index, index);

    palettes.insert(palettes.begin() + index, pal);
    _palettePixmaps.insert(index, palettePixmap(index));

    endInsertRows();

    emit _document->paletteAdded(index);
}

void PalettesModel::removePalette(unsigned index)
{
    Q_ASSERT(_document != nullptr);

    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(index < palettes.size());

    emit _document->paletteAboutToBeRemoved(index);

    beginRemoveRows(QModelIndex(), index, index);

    palettes.erase(palettes.begin() + index);
    _palettePixmaps.removeAt(index);

    endRemoveRows();
}

void PalettesModel::raisePalette(unsigned index)
{
    Q_ASSERT(_document != nullptr);
    Q_ASSERT(index != 0);

    auto& palettes = _document->frameSet()->palettes;

    beginMoveRows(QModelIndex(), index, index, QModelIndex(), index - 1);

    std::swap(palettes.at(index),
              palettes.at(index - 1));
    _palettePixmaps.swap(index, index - 1);

    endMoveRows();

    emit _document->paletteMoved(index, index - 1);
}

void PalettesModel::lowerPalette(unsigned index)
{
    Q_ASSERT(_document != nullptr);

    auto& palettes = _document->frameSet()->palettes;
    Q_ASSERT(index < palettes.size() - 1);

    beginMoveRows(QModelIndex(), index, index, QModelIndex(), index + 2);

    std::swap(palettes.at(index),
              palettes.at(index + 1));
    _palettePixmaps.swap(index, index + 1);

    endMoveRows();

    emit _document->paletteMoved(index, index + 1);
}
