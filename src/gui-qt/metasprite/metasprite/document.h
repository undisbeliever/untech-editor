/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "models/metasprite/metasprite.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {

class SmallTileTileset;
class LargeTileTileset;
class PaletteList;
class FrameMap;
class FrameObjectList;
class ActionPointList;
class EntityHitboxList;

namespace MS = UnTech::MetaSprite::MetaSprite;

class Document : public AbstractMsDocument {
    Q_OBJECT

public:
    using DataT = MS::FrameSet;

    using TilesetType = UnTech::MetaSprite::TilesetType;

public:
    Document(FrameSetResourceList* parent, size_t index);
    ~Document() = default;

    static QString typeName() { return tr("MetaSprite FrameSet"); }

    MS::FrameSet* frameSet() const { return _frameSet; }
    virtual MSA::Animation::map_t* animations() const final { return &_frameSet->animations; }

    virtual QStringList frameNames() const final;

    SmallTileTileset* smallTileTileset() const { return _smallTileTileset; }
    LargeTileTileset* largeTileTileset() const { return _largeTileTileset; }
    PaletteList* paletteList() const { return _paletteList; }
    FrameMap* frameMap() const { return _frameMap; }
    FrameObjectList* frameObjectList() const { return _frameObjectList; }
    ActionPointList* actionPointList() const { return _actionPointList; }
    EntityHitboxList* entityHitboxList() const { return _entityHitboxList; }

    bool editFrameSet_setName(const idstring& name);
    bool editFrameSet_setTilesetType(TilesetType ts);
    bool editFrameSet_setExportOrder(const idstring& exportOrder);

private:
    friend class Accessor::ResourceItemUndoHelper<Document>;
    MS::FrameSet* data() const { return _frameSet; }
    MS::FrameSet* dataEditable() { return _frameSet; }

protected:
    // can throw exceptions
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(RES::ErrorList& err) final;
    virtual bool compileResource(RES::ErrorList& err) final;

private slots:
    void onFrameSetNameChanged();
    void onFrameSetExportOrderChanged();

private:
    void resetDocumentState();

private:
    MS::FrameSet* _frameSet;

    SmallTileTileset* const _smallTileTileset;
    LargeTileTileset* const _largeTileTileset;
    PaletteList* const _paletteList;
    FrameMap* const _frameMap;
    FrameObjectList* const _frameObjectList;
    ActionPointList* const _actionPointList;
    EntityHitboxList* const _entityHitboxList;
};

}
}
}
}
