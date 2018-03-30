/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
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
    Document* const _document;
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

class MovePalette : public QUndoCommand {
public:
    MovePalette(Document* document, unsigned fromIndex, unsigned toIndex);
    MovePalette(Document* document, unsigned fromIndex, unsigned toIndex,
                const QString& text);
    ~MovePalette() = default;

    virtual void undo() final;
    virtual void redo() final;

private:
    Document* const _document;
    const unsigned _fromIndex;
    const unsigned _toIndex;
};

class RaisePalette : public MovePalette {
public:
    RaisePalette(Document* document, unsigned index);
    ~RaisePalette() = default;
};

class LowerPalette : public MovePalette {
public:
    LowerPalette(Document* document, unsigned index);
    ~LowerPalette() = default;
};

class ChangePaletteColor : public QUndoCommand {
public:
    ChangePaletteColor(Document* document, unsigned paletteIndex,
                       unsigned colorIndex);
    ChangePaletteColor(Document* document, unsigned paletteIndex,
                       unsigned colorIndex, const Snes::SnesColor& color);
    ~ChangePaletteColor() = default;

    void setNewColor(const Snes::SnesColor& color);

    virtual void undo() final;
    virtual void redo() final;

private:
    void doChangePaletteColor(const Snes::SnesColor& color);

private:
    Document* const _document;
    const unsigned _paletteIndex;
    const unsigned _colorIndex;
    const Snes::SnesColor _oldColor;
    Snes::SnesColor _newColor;
};
}
}
}
}
