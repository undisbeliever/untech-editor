/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "models/metasprite/spriteimporter.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {

class FrameMap;
class FrameObjectList;
class ActionPointList;
class EntityHitboxList;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Document : public AbstractMsDocument {
    Q_OBJECT

public:
    using DataT = SI::FrameSet;

    using TilesetType = UnTech::MetaSprite::TilesetType;

public:
    Document(FrameSetResourceList* parent, size_t index);
    ~Document() = default;

    static QString typeName() { return tr("SpriteImporter FrameSet"); }

    SI::FrameSet* frameSet() const { return _frameSet; }
    virtual MSA::Animation::map_t* animations() const final { return &_frameSet->animations; }

    virtual QStringList frameNames() const final;
    virtual unsigned nPalettes() const final;

    FrameMap* frameMap() const { return _frameMap; }
    FrameObjectList* frameObjectList() const { return _frameObjectList; }
    ActionPointList* actionPointList() const { return _actionPointList; }
    EntityHitboxList* entityHitboxList() const { return _entityHitboxList; }

    bool editFrameSet_setName(const idstring& name);
    bool editFrameSet_setTilesetType(TilesetType ts);
    bool editFrameSet_setExportOrder(const idstring& exportOrder);
    bool editFrameSet_setImageFilename(const std::string& filename);
    bool editFrameSet_setTransparentColor(const rgba& color);
    bool editFrameSet_setGrid(const SI::FrameSetGrid& grid);
    bool editFrameSet_setPalette(const SI::UserSuppliedPalette& palette);

private:
    friend class Accessor::ResourceItemUndoHelper<Document>;
    const SI::FrameSet* data() const { return _frameSet; }
    SI::FrameSet* dataEditable() { return _frameSet; }

protected:
    // can throw exceptions
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(ErrorList& err) final;
    virtual bool compileResource(ErrorList& err) final;

private slots:
    void onFrameSetExportOrderChanged();
    void onFrameSetImageFilenameChanged();

private:
    void resetDocumentState();

signals:
    void frameSetGridChanged();
    void frameSetImageFilenameChanged();
    void frameSetPaletteChanged();

private:
    SI::FrameSet* _frameSet;

    FrameMap* const _frameMap;
    FrameObjectList* const _frameObjectList;
    ActionPointList* const _actionPointList;
    EntityHitboxList* const _entityHitboxList;
};
}
}
}
}
