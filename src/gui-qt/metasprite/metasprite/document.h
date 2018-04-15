/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "selection.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "models/metasprite/metasprite.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {

class PaletteList;

namespace MS = UnTech::MetaSprite::MetaSprite;

class Document : public AbstractMsDocument {
    Q_OBJECT

public:
    using FrameT = MS::Frame;

public:
    Document(FrameSetResourceList* parent, size_t index);
    ~Document() = default;

    MS::FrameSet* frameSet() const { return _frameSet; }
    virtual MSA::Animation::map_t* animations() const final { return &_frameSet->animations; }

    virtual Selection* selection() const final { return _selection; }

    virtual QStringList frameList() const final;

    PaletteList* paletteList() const { return _paletteList; }

protected:
    // can throw exceptions
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(RES::ErrorList& err) final;
    virtual bool compileResource(RES::ErrorList& err) final;

private slots:
    void onFrameSetNameChanged();

private:
    void resetDocumentState();

signals:
    void smallTilesetChanged();
    void largeTilesetChanged();

    void smallTileChanged(unsigned tileId);
    void largeTileChanged(unsigned tileId);

private:
    MS::FrameSet* _frameSet;

    Selection* const _selection;

    PaletteList* const _paletteList;
};
}
}
}
}
