/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetdock.h"
#include "document.h"
#include "framecontentcommands.h"
#include "selection.h"
#include "tilesetcommands.h"
#include "tilesetwidgets.h"
#include "gui-qt/metasprite/metasprite/tilesetdock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

TilesetDock::TilesetDock(TilesetPixmaps* tilesetPixmaps, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::TilesetDock)
    , _document(nullptr)
{
    Q_ASSERT(tilesetPixmaps != nullptr);

    _ui->setupUi(this);

    _ui->smallTileset->setTilesetPixmaps(tilesetPixmaps);
    _ui->largeTileset->setTilesetPixmaps(tilesetPixmaps);

    _ui->scrollAreaContents->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(_ui->smallTileset, &DrawingPixmapGridWidget::cellClicked,
            [this](int index) { onTileClicked(ObjectSize::SMALL, index); });
    connect(_ui->largeTileset, &DrawingPixmapGridWidget::cellClicked,
            [this](int index) { onTileClicked(ObjectSize::LARGE, index); });

    connect(_ui->scrollAreaContents, &QWidget::customContextMenuRequested,
            this, &TilesetDock::onContextMenu);
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

    _ui->smallTileset->setDocument(_document);
    _ui->largeTileset->setDocument(_document);

    if (_document) {
        onSelectedItemsChanged();

        connect(_document, &Document::frameObjectChanged,
                this, &TilesetDock::onFrameObjectChanged);

        connect(_document->selection(), &Selection::selectedItemsChanged,
                this, &TilesetDock::onSelectedItemsChanged);
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
    if (_document->selection()->selectedColor() >= 0) {
        // don't change tile in edit tiles mode
        return;
    }

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

void TilesetDock::onContextMenu(const QPoint& pos)
{
    if (_document) {
        QPoint globalPos = _ui->scrollAreaContents->mapToGlobal(pos);

        int index = -1;
        bool isSmall = false;

        if (pos.y() < _ui->line->y() + _ui->line->height() / 2) {
            isSmall = true;
            index = _ui->smallTileset->indexAt(_ui->smallTileset->mapFromGlobal(globalPos));
        }
        else {
            isSmall = false;
            index = _ui->largeTileset->indexAt(_ui->largeTileset->mapFromGlobal(globalPos));
        }

        QMenu menu;
        auto addAction = [&](const char* text, auto functor) {
            QAction* action = menu.addAction(tr(text));
            connect(action, &QAction::triggered, functor);
        };

        if (index >= 0) {
            if (isSmall) {
                addAction("Add Small Tile", [=]() {
                    _document->undoStack()->push(
                        new AddSmallTile(_document));
                });
                addAction("Add Small Tile Here", [=]() {
                    _document->undoStack()->push(
                        new AddSmallTile(_document, index));
                });
                addAction("Clone Small Tile Here", [=]() {
                    _document->undoStack()->push(
                        new CloneSmallTile(_document, index));
                });
                addAction("Remove Small Tile", [=]() {
                    _document->undoStack()->push(
                        new RemoveSmallTile(_document, index));
                });
            }
            else {
                addAction("Add Large Tile", [=]() {
                    _document->undoStack()->push(
                        new AddLargeTile(_document));
                });
                addAction("Add Large Tile Here", [=]() {
                    _document->undoStack()->push(
                        new AddLargeTile(_document, index));
                });
                addAction("Clone Large Tile Here", [=]() {
                    _document->undoStack()->push(
                        new CloneLargeTile(_document, index));
                });
                addAction("Remove Large Tile", [=]() {
                    _document->undoStack()->push(
                        new RemoveLargeTile(_document, index));
                });
            }
        }
        else {
            addAction("Add Small Tile", [=]() {
                _document->undoStack()->push(
                    new AddSmallTile(_document));
            });
            addAction("Add Large Tile", [=]() {
                _document->undoStack()->push(
                    new AddLargeTile(_document));
            });
        }

        menu.exec(globalPos);
    }
}
