/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetdock.h"
#include "accessors.h"
#include "resourceitem.h"
#include "tilesetwidgets.h"
#include "gui-qt/metasprite/metasprite/tilesetdock.ui.h"

#include <QMenu>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

TilesetDock::TilesetDock(TilesetPixmaps* tilesetPixmaps, QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::TilesetDock)
    , _resourceItem(nullptr)
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

void TilesetDock::setResourceItem(ResourceItem* resourceItem)
{
    if (_resourceItem == resourceItem) {
        return;
    }

    if (_resourceItem != nullptr) {
        _resourceItem->frameObjectList()->disconnect(this);
    }
    _resourceItem = resourceItem;

    setEnabled(_resourceItem != nullptr);

    _ui->smallTileset->setResourceItem(_resourceItem);
    _ui->largeTileset->setResourceItem(_resourceItem);

    if (_resourceItem) {
        onSelectedFrameObjectsChanged();

        connect(_resourceItem->frameObjectList(), &FrameObjectList::dataChanged,
                this, &TilesetDock::onSelectedFrameObjectsChanged);

        connect(_resourceItem->frameObjectList(), &FrameObjectList::selectedIndexesChanged,
                this, &TilesetDock::onSelectedFrameObjectsChanged);
    }
}

void TilesetDock::onSelectedFrameObjectsChanged()
{
    // To simplify the UI the selected tile will only be shown if
    // selectedItems contains a single Frame Object.

    const int index = selectedFrameObjectIndex();
    if (index >= 0) {
        const MS::Frame* frame = _resourceItem->frameList()->selectedItem();
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
    const MS::Frame* frame = _resourceItem->frameList()->selectedItem();
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
    // do not change tile in edit tiles mode
    if (_resourceItem->paletteList()->isSelectedColorValid() == false) {
        _resourceItem->frameObjectList()->editSelected_setTileIdAndSize(tileIndex, size);
    }
}

int TilesetDock::selectedFrameObjectIndex() const
{
    const auto& selectedIndexes = _resourceItem->frameObjectList()->selectedIndexes();
    if (selectedIndexes.size() == 1) {
        return selectedIndexes.front();
    }
    else {
        return -1;
    }
}

void TilesetDock::onContextMenu(const QPoint& pos)
{
    if (_resourceItem && _resourceItem->frameSet()) {
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
        auto addAction = [&](const char* text, auto&& functor) {
            QAction* action = menu.addAction(tr(text));
            connect(action, &QAction::triggered, functor);
        };

        if (index >= 0) {
            if (isSmall) {
                addAction("Add Small Tile", [=]() {
                    _resourceItem->smallTileTileset()->addItem();
                });
                addAction("Add Small Tile Here", [=]() {
                    _resourceItem->smallTileTileset()->addItem(index);
                });
                addAction("Clone Small Tile Here", [=]() {
                    _resourceItem->smallTileTileset()->cloneItem(index);
                });
                addAction("Remove Small Tile", [=]() {
                    _resourceItem->smallTileTileset()->removeItem(index);
                });
            }
            else {
                addAction("Add Large Tile", [=]() {
                    _resourceItem->largeTileTileset()->addItem();
                });
                addAction("Add Large Tile Here", [=]() {
                    _resourceItem->largeTileTileset()->addItem(index);
                });
                addAction("Clone Large Tile Here", [=]() {
                    _resourceItem->largeTileTileset()->cloneItem(index);
                });
                addAction("Remove Large Tile", [=]() {
                    _resourceItem->largeTileTileset()->removeItem(index);
                });
            }
        }
        else {
            addAction("Add Small Tile", [=]() {
                _resourceItem->smallTileTileset()->addItem();
            });
            addAction("Add Large Tile", [=]() {
                _resourceItem->largeTileTileset()->addItem();
            });
        }

        menu.exec(globalPos);
    }
}
