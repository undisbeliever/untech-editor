/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetdock.h"
#include "document.h"
#include "selection.h"
#include "tilesetpixmaps.h"
#include "gui-qt/metasprite/metasprite/tilesetdock.ui.h"

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

TilesetDock::TilesetDock(TilesetPixmaps* tilesetPixmaps, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::TilesetDock)
    , _tilesetPixmaps(tilesetPixmaps)
    , _document(nullptr)
{
    Q_ASSERT(tilesetPixmaps != nullptr);

    _ui->setupUi(this);

    _ui->smallTileset->setCellSize(QSize(8 * 6, 8 * 6));
    _ui->largeTileset->setCellSize(QSize(16 * 6, 16 * 6));

    connect(_tilesetPixmaps, &TilesetPixmaps::pixmapsChanged,
            this, &TilesetDock::onTilesetPixmapChanged);
}

TilesetDock::~TilesetDock() = default;

void TilesetDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
        _document->selection()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    if (_document) {
        updateBackgroundColor();

        connect(_document, &Document::paletteChanged,
                this, &TilesetDock::onPaletteChanged);

        connect(_document->selection(), &Selection::selectedPaletteChanged,
                this, &TilesetDock::updateBackgroundColor);
    }
}

void TilesetDock::onTilesetPixmapChanged()
{
    _ui->smallTileset->setPixmaps(_tilesetPixmaps->smallTileset());
    _ui->largeTileset->setPixmaps(_tilesetPixmaps->largeTileset());
}

void TilesetDock::onPaletteChanged(unsigned index)
{
    if (index == _document->selection()->selectedPalette()) {
        updateBackgroundColor();
    }
}

void TilesetDock::updateBackgroundColor()
{
    const auto& palettes = _document->frameSet()->palettes;
    const unsigned selected = _document->selection()->selectedPalette();

    if (selected < palettes.size()) {
        const auto& rgb = palettes.at(selected).color(0).rgb();
        QColor bg = qRgb(rgb.red, rgb.green, rgb.blue);

        _ui->smallTileset->setBackgroundColor(bg);
        _ui->largeTileset->setBackgroundColor(bg);
    }
}
