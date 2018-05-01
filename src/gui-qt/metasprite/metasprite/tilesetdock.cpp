/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetdock.h"
#include "accessors.h"
#include "document.h"
#include "tilesetwidgets.h"
#include "gui-qt/accessor/listandmultipleselectionundohelper.h"
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
        _document->frameObjectList()->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    _ui->smallTileset->setDocument(_document);
    _ui->largeTileset->setDocument(_document);

    if (_document) {
        onSelectedFrameObjectsChanged();

        connect(_document->frameObjectList(), &FrameObjectList::dataChanged,
                this, &TilesetDock::onSelectedFrameObjectsChanged);

        connect(_document->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &TilesetDock::onSelectedFrameObjectsChanged);
    }
}

void TilesetDock::onSelectedFrameObjectsChanged()
{
    // To simplify the UI the selected tile will only be shown if
    // selectedItems contains a single Frame Object.

    const int index = selectedFrameObjectIndex();
    if (index >= 0) {
        const MS::Frame* frame = _document->frameMap()->selectedFrame();
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
    const MS::Frame* frame = _document->frameMap()->selectedFrame();
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
    if (_document->paletteList()->isSelectedColorValid()) {
        // don't change tile in edit tiles mode
        return;
    }

    const int index = selectedFrameObjectIndex();
    if (index >= 0) {
        const MS::Frame* frame = _document->frameMap()->selectedFrame();
        MS::FrameObject obj = frame->objects.at(index);

        obj.size = size;
        obj.tileId = tileIndex;

        FrameObjectListUndoHelper(_document->frameObjectList())
            .editItemInSelectedList(index, obj);
    }
}

int TilesetDock::selectedFrameObjectIndex() const
{
    const auto& selectedIndexes = _document->frameObjectList()->selectedIndexes();
    if (selectedIndexes.size() == 1) {
        return selectedIndexes.front();
    }
    else {
        return -1;
    }
}

void TilesetDock::pushAddTileCommand(QUndoCommand* c, unsigned tileId, ObjectSize size)
{
    if (c) {
        _document->undoStack()->beginMacro(c->text());
        _document->undoStack()->push(c);

        FrameObjectListUndoHelper(_document->frameObjectList())
            .editAllItemsInSelectedList(QString(), [&](MS::FrameObject& obj, size_t) {
                if (obj.size == size
                    && obj.tileId >= tileId) {

                    obj.tileId++;
                }
            });

        _document->undoStack()->endMacro();
    }
}

void TilesetDock::pushRemoveTileCommand(QUndoCommand* c, unsigned tileId, ObjectSize size)
{
    if (c) {
        _document->undoStack()->beginMacro(c->text());
        _document->undoStack()->push(c);

        FrameObjectListUndoHelper(_document->frameObjectList())
            .editAllItemsInSelectedList(QString(), [&](MS::FrameObject& obj, size_t) {
                if (obj.size == size
                    && obj.tileId >= tileId
                    && obj.tileId > 0) {

                    obj.tileId--;
                }
            });

        _document->undoStack()->endMacro();
    }
}

void TilesetDock::onContextMenu(const QPoint& pos)
{
    if (_document && _document->frameSet()) {
        QPoint globalPos = _ui->scrollAreaContents->mapToGlobal(pos);

        MS::FrameSet* fs = _document->frameSet();

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

        auto addSmallTile = [this](unsigned tileId) {
            QUndoCommand* c = SmallTileTilesetUndoHelper(_document->smallTileTileset())
                                  .addCommand(std::make_tuple(), tileId);
            pushAddTileCommand(c, tileId, ObjectSize::SMALL);
        };
        auto addLargeTile = [this](unsigned tileId) {
            QUndoCommand* c = LargeTileTilesetUndoHelper(_document->largeTileTileset())
                                  .addCommand(std::make_tuple(), tileId);
            pushAddTileCommand(c, tileId, ObjectSize::LARGE);
        };

        QMenu menu;
        auto addAction = [&](const char* text, auto&& functor) {
            QAction* action = menu.addAction(tr(text));
            connect(action, &QAction::triggered, functor);
        };

        if (index >= 0) {
            if (isSmall) {
                addAction("Add Small Tile", [=]() {
                    addSmallTile(fs->smallTileset.size());
                });
                addAction("Add Small Tile Here", [=]() {
                    addSmallTile(index);
                });
                addAction("Clone Small Tile Here", [=]() {
                    QUndoCommand* c = SmallTileTilesetUndoHelper(_document->smallTileTileset())
                                          .cloneCommand(std::make_tuple(), index);
                    pushAddTileCommand(c, index, ObjectSize::SMALL);
                });
                addAction("Remove Small Tile", [=]() {
                    QUndoCommand* c = SmallTileTilesetUndoHelper(_document->smallTileTileset())
                                          .removeCommand(std::make_tuple(), index);
                    pushRemoveTileCommand(c, index, ObjectSize::SMALL);
                });
            }
            else {
                addAction("Add Large Tile", [=]() {
                    addLargeTile(fs->largeTileset.size());
                });
                addAction("Add Large Tile Here", [=]() {
                    addLargeTile(index);
                });
                addAction("Clone Large Tile Here", [=]() {
                    QUndoCommand* c = LargeTileTilesetUndoHelper(_document->largeTileTileset())
                                          .cloneCommand(std::make_tuple(), index);
                    pushAddTileCommand(c, index, ObjectSize::LARGE);
                });
                addAction("Remove Large Tile", [=]() {
                    QUndoCommand* c = LargeTileTilesetUndoHelper(_document->largeTileTileset())
                                          .removeCommand(std::make_tuple(), index);
                    pushRemoveTileCommand(c, index, ObjectSize::LARGE);
                });
            }
        }
        else {
            addAction("Add Small Tile", [=]() {
                addSmallTile(fs->smallTileset.size());
            });
            addAction("Add Large Tile", [=]() {
                addLargeTile(fs->largeTileset.size());
            });
        }

        menu.exec(globalPos);
    }
}
