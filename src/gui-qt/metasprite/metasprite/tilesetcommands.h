/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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
}
}
}
}
