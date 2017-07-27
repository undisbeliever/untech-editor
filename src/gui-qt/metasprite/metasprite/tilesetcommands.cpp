/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tilesetcommands.h"
#include "document.h"

#include <QCoreApplication>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using namespace UnTech::Snes;

// Change Tile
// ===========
const int ChangeSmallTile::ID = 0x711e5e70;
const int ChangeLargeTile::ID = 0x711e5e71;

#define CHANGE_TILE(CLASS, TILE_CLASS, TILESET, SIGNAL)          \
                                                                 \
    CLASS::CLASS(Document* document, unsigned tileId,            \
                 const Snes::TILE_CLASS& tile, bool first)       \
        : QUndoCommand(QCoreApplication::tr("Change Tile"))      \
        , _document(document)                                    \
        , _tileId(tileId)                                        \
        , _oldTile(document->frameSet()->TILESET.tile(tileId))   \
        , _newTile(tile)                                         \
        , _first(first)                                          \
    {                                                            \
        Q_ASSERT(_oldTile != _newTile);                          \
    }                                                            \
                                                                 \
    void CLASS::undo()                                           \
    {                                                            \
        _document->frameSet()->TILESET.tile(_tileId) = _oldTile; \
        emit _document->SIGNAL(_tileId);                         \
    }                                                            \
                                                                 \
    void CLASS::redo()                                           \
    {                                                            \
        _document->frameSet()->TILESET.tile(_tileId) = _newTile; \
        emit _document->SIGNAL(_tileId);                         \
    }                                                            \
                                                                 \
    int CLASS::id() const                                        \
    {                                                            \
        return ID;                                               \
    }                                                            \
                                                                 \
    bool CLASS::mergeWith(const QUndoCommand* cmd)               \
    {                                                            \
        const CLASS* command = dynamic_cast<const CLASS*>(cmd);  \
        if (command                                              \
            && command->_first == false                          \
            && this->_tileId == command->_tileId) {              \
                                                                 \
            _newTile = command->_newTile;                        \
            return true;                                         \
        }                                                        \
        else {                                                   \
            return false;                                        \
        }                                                        \
    }
CHANGE_TILE(ChangeSmallTile, Tile8px, smallTileset, smallTileChanged)
CHANGE_TILE(ChangeLargeTile, Tile16px, largeTileset, largeTileChanged)
