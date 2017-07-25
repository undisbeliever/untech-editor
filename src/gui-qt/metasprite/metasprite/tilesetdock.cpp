/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetdock.h"
#include "document.h"
#include "framecontentcommands.h"
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

    connect(_ui->smallTileset, &PixmapGridWidget::cellClicked,
            [this](int index) { onTileClicked(ObjectSize::SMALL, index); });
    connect(_ui->largeTileset, &PixmapGridWidget::cellClicked,
            [this](int index) { onTileClicked(ObjectSize::LARGE, index); });
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
        onSelectedItemsChanged();

        connect(_document, &Document::paletteChanged,
                this, &TilesetDock::onPaletteChanged);

        connect(_document, &Document::frameObjectChanged,
                this, &TilesetDock::onFrameObjectChanged);

        connect(_document->selection(), &Selection::selectedPaletteChanged,
                this, &TilesetDock::updateBackgroundColor);

        connect(_document->selection(), &Selection::selectedItemsChanged,
                this, &TilesetDock::onSelectedItemsChanged);
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

void TilesetDock::onSelectedItemsChanged()
{
    // To simplify the UI the selected tile will only be shown if
    // selectedItems contains a single Frame Object.

    const int index = selectedFrameObjectIndex();
    if (index >= 0) {
        const MS::Frame* frame = _document->selection()->selectedFrame();
        const MS::FrameObject& obj = frame->objects.at(index);

        if (obj.size == ObjectSize::SMALL) {
            _ui->smallTileset->setSelected(obj.tileId);
            _ui->largeTileset->clearSelected();
        }
        else {
            _ui->largeTileset->setSelected(obj.tileId);
            _ui->smallTileset->clearSelected();
        }
    }
    else {
        _ui->smallTileset->clearSelected();
        _ui->largeTileset->clearSelected();
    }
}

void TilesetDock::onFrameObjectChanged(const void* changedFrame, unsigned changedIndex)
{
    const MS::Frame* frame = _document->selection()->selectedFrame();
    if (frame == changedFrame) {
        const int index = selectedFrameObjectIndex();

        if (index >= 0 && int(changedIndex) == index) {
            const MS::FrameObject& obj = frame->objects.at(index);

            if (obj.size == ObjectSize::SMALL) {
                _ui->smallTileset->setSelected(obj.tileId);
                _ui->largeTileset->clearSelected();
            }
            else {
                _ui->largeTileset->setSelected(obj.tileId);
                _ui->smallTileset->clearSelected();
            }
        }
    }
}

void TilesetDock::onTileClicked(ObjectSize size, int tileIndex)
{
    const int index = selectedFrameObjectIndex();
    if (index >= 0) {
        MS::Frame* frame = _document->selection()->selectedFrame();
        const MS::FrameObject& oldObj = frame->objects.at(index);
        MS::FrameObject obj = oldObj;

        obj.size = size;
        obj.tileId = tileIndex;

        if (obj != oldObj) {
            _document->undoStack()->push(
                new ChangeFrameObject(_document, frame, index, obj));
        }
    }
}

int TilesetDock::selectedFrameObjectIndex() const
{
    const auto& selectedItems = _document->selection()->selectedItems();
    if (selectedItems.size() != 1) {
        return -1;
    }

    const auto& it = selectedItems.cbegin();
    if (it->type != SelectedItem::FRAME_OBJECT) {
        return -1;
    }

    return it->index;
}
