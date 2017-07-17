/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/snes/palette.h"
#include <QUndoCommand>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;

class AddRemovePalette : public QUndoCommand {
protected:
    AddRemovePalette(Document* document,
                     unsigned index, const Snes::Palette4bpp&,
                     const QString& text);
    ~AddRemovePalette() = default;

protected:
    void addPalette();
    void removePalette();

private:
    Document* _document;
    const unsigned _index;
    const Snes::Palette4bpp _palette;
};

class AddPalette : public AddRemovePalette {
public:
    AddPalette(Document* document);
    ~AddPalette() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class ClonePalette : public AddRemovePalette {
public:
    ClonePalette(Document* document, unsigned index);
    ~ClonePalette() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RemovePalette : public AddRemovePalette {
public:
    RemovePalette(Document* document, unsigned index);
    ~RemovePalette() = default;

    virtual void undo() final;
    virtual void redo() final;
};

class RaisePalette : public QUndoCommand {
public:
    RaisePalette(Document* document, unsigned index);
    ~RaisePalette() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    const unsigned _index;
};

class LowerPalette : public QUndoCommand {
public:
    LowerPalette(Document* document, unsigned index);
    ~LowerPalette() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* _document;
    const unsigned _index;
};
}
}
}
}
