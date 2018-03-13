/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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

#define ADD_REMOVE_TILE_COMMAND(CLS, TILE_CLASS, TILESET, SIZE, SIGNAL)     \
    AddRemove##CLS::AddRemove##CLS(Document* document,                      \
                                   unsigned tileId, const TILE_CLASS& tile, \
                                   const QString& text)                     \
        : QUndoCommand(text)                                                \
        , _document(document)                                               \
        , _tileId(tileId)                                                   \
        , _tile(tile)                                                       \
    {                                                                       \
    }                                                                       \
                                                                            \
    void AddRemove##CLS::addTile()                                          \
    {                                                                       \
        _document->frameSet()->TILESET.addTile(_tileId, _tile);             \
        emit _document->SIGNAL();                                           \
                                                                            \
        updateFrameObjects(+1);                                             \
    }                                                                       \
                                                                            \
    void AddRemove##CLS::removeTile()                                       \
    {                                                                       \
        _document->frameSet()->TILESET.removeTile(_tileId);                 \
        emit _document->SIGNAL();                                           \
                                                                            \
        updateFrameObjects(-1);                                             \
    }                                                                       \
                                                                            \
    void AddRemove##CLS::generateFrameObjectsVector(unsigned minTileId)     \
    {                                                                       \
        using ObjSize = UnTech::MetaSprite::ObjectSize;                     \
                                                                            \
        Q_ASSERT(_frameObjects.empty());                                    \
                                                                            \
        for (const auto& it : _document->frameSet()->frames) {              \
            MS::Frame& frame = it.second;                                   \
                                                                            \
            for (unsigned i = 0; i < frame.objects.size(); i++) {           \
                const MS::FrameObject& obj = frame.objects.at(i);           \
                                                                            \
                if (obj.size == ObjSize::SIZE && obj.tileId >= minTileId) { \
                    _frameObjects.emplace_back(&frame, i);                  \
                }                                                           \
            }                                                               \
        }                                                                   \
    }                                                                       \
                                                                            \
    void AddRemove##CLS::updateFrameObjects(int offset)                     \
    {                                                                       \
        for (const auto& obj : _frameObjects) {                             \
            MS::Frame* frame = obj.first;                                   \
            unsigned index = obj.second;                                    \
                                                                            \
            frame->objects.at(index).tileId += offset;                      \
            emit _document->frameObjectChanged(frame, index);               \
        }                                                                   \
    }
ADD_REMOVE_TILE_COMMAND(SmallTile, Tile8px, smallTileset, SMALL, smallTilesetChanged);
ADD_REMOVE_TILE_COMMAND(LargeTile, Tile16px, largeTileset, LARGE, largeTilesetChanged);

#define ADD_TILE_COMMAND(CLS, TILE_CLASS, TILESET, TEXT)           \
    Add##CLS::Add##CLS(Document* document)                         \
        : Add##CLS(document, document->frameSet()->TILESET.size()) \
    {                                                              \
    }                                                              \
                                                                   \
    Add##CLS::Add##CLS(Document* document, unsigned tileId)        \
        : AddRemove##CLS(document, tileId, TILE_CLASS(),           \
                         QCoreApplication::tr(TEXT))               \
    {                                                              \
        generateFrameObjectsVector(tileId);                        \
    }                                                              \
                                                                   \
    void Add##CLS::undo()                                          \
    {                                                              \
        removeTile();                                              \
    }                                                              \
    void Add##CLS::redo()                                          \
    {                                                              \
        addTile();                                                 \
    }
ADD_TILE_COMMAND(SmallTile, Tile8px, smallTileset, "Add Small Tile");
ADD_TILE_COMMAND(LargeTile, Tile16px, largeTileset, "Add Large Tile");

#define CLONE_TILE_COMMAND(CLS, TILESET, TEXT)                       \
    Clone##CLS::Clone##CLS(Document* document, unsigned tileId)      \
        : AddRemove##CLS(document, tileId,                           \
                         document->frameSet()->TILESET.tile(tileId), \
                         QCoreApplication::tr(TEXT))                 \
    {                                                                \
        generateFrameObjectsVector(tileId + 1);                      \
    }                                                                \
                                                                     \
    void Clone##CLS::undo()                                          \
    {                                                                \
        removeTile();                                                \
    }                                                                \
    void Clone##CLS::redo()                                          \
    {                                                                \
        addTile();                                                   \
    }
CLONE_TILE_COMMAND(SmallTile, smallTileset, "Clone Small Tile");
CLONE_TILE_COMMAND(LargeTile, largeTileset, "Clone Large Tile");

#define REMOVE_TILE_COMMAND(CLS, TILESET, TEXT)                      \
    Remove##CLS::Remove##CLS(Document* document, unsigned tileId)    \
        : AddRemove##CLS(document, tileId,                           \
                         document->frameSet()->TILESET.tile(tileId), \
                         QCoreApplication::tr(TEXT))                 \
    {                                                                \
        generateFrameObjectsVector(tileId + 1);                      \
    }                                                                \
                                                                     \
    void Remove##CLS::undo()                                         \
    {                                                                \
        addTile();                                                   \
    }                                                                \
    void Remove##CLS::redo()                                         \
    {                                                                \
        removeTile();                                                \
    }
REMOVE_TILE_COMMAND(SmallTile, smallTileset, "Remove Small Tile");
REMOVE_TILE_COMMAND(LargeTile, largeTileset, "Remove Large Tile");
