/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/metasprite.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;

namespace MS = UnTech::MetaSprite::MetaSprite;

#define CHANGE_TILE(CLASS, TILE_CLASS)                             \
    class CLASS : public QUndoCommand {                            \
        const static int ID;                                       \
                                                                   \
    public:                                                        \
        CLASS(Document* document, unsigned tileId,                 \
              const TILE_CLASS& tile, bool first);                 \
        ~CLASS() = default;                                        \
                                                                   \
        virtual void undo() final;                                 \
        virtual void redo() final;                                 \
                                                                   \
        virtual int id() const final;                              \
        virtual bool mergeWith(const QUndoCommand* command) final; \
                                                                   \
    private:                                                       \
        Document* _document;                                       \
        unsigned _tileId;                                          \
        const TILE_CLASS _oldTile;                                 \
        TILE_CLASS _newTile;                                       \
        const bool _first;                                         \
    };
CHANGE_TILE(ChangeSmallTile, UnTech::Snes::Tile8px)
CHANGE_TILE(ChangeLargeTile, UnTech::Snes::Tile16px)

#undef CHANGE_TILE

#define ADD_REMOVE_TILE(CLS, TILE_CLASS)                            \
    class AddRemove##CLS : public QUndoCommand {                    \
    protected:                                                      \
        AddRemove##CLS(Document* document,                          \
                       unsigned tileId, const TILE_CLASS&,          \
                       const QString& text);                        \
        ~AddRemove##CLS() = default;                                \
                                                                    \
    protected:                                                      \
        void generateFrameObjectsVector(unsigned minTileId);        \
                                                                    \
        void addTile();                                             \
        void removeTile();                                          \
                                                                    \
    private:                                                        \
        void updateFrameObjects(int offset);                        \
                                                                    \
    private:                                                        \
        Document* _document;                                        \
        const unsigned _tileId;                                     \
        const TILE_CLASS _tile;                                     \
        std::vector<std::pair<MS::Frame*, unsigned>> _frameObjects; \
    };                                                              \
                                                                    \
    class Add##CLS : public AddRemove##CLS {                        \
    public:                                                         \
        Add##CLS(Document* document);                               \
        Add##CLS(Document* document, unsigned tileId);              \
        ~Add##CLS() = default;                                      \
                                                                    \
        virtual void undo() final;                                  \
        virtual void redo() final;                                  \
    };                                                              \
                                                                    \
    class Clone##CLS : public AddRemove##CLS {                      \
    public:                                                         \
        Clone##CLS(Document* document, unsigned tileId);            \
        ~Clone##CLS() = default;                                    \
                                                                    \
        virtual void undo() final;                                  \
        virtual void redo() final;                                  \
    };                                                              \
                                                                    \
    class Remove##CLS : public AddRemove##CLS {                     \
    public:                                                         \
        Remove##CLS(Document* document, unsigned tileId);           \
        ~Remove##CLS() = default;                                   \
                                                                    \
        virtual void undo() final;                                  \
        virtual void redo() final;                                  \
    };
ADD_REMOVE_TILE(SmallTile, UnTech::Snes::Tile8px)
ADD_REMOVE_TILE(LargeTile, UnTech::Snes::Tile16px)

#undef ADD_REMOVE_TILE
}
}
}
}
