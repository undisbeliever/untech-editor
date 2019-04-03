/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourceitem.h"
#include "accessors.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/animation/accessors.h"

using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

ResourceItem::ResourceItem(FrameSetResourceList* parent, size_t index)
    : AbstractMsResourceItem(parent, index)
    , _frameSet(nullptr)
    , _smallTileTileset(new SmallTileTileset(this))
    , _largeTileTileset(new LargeTileTileset(this))
    , _paletteList(new PaletteList(this))
    , _frameList(new FrameList(this))
    , _frameObjectList(new FrameObjectList(this))
    , _actionPointList(new ActionPointList(this))
    , _entityHitboxList(new EntityHitboxList(this))
{
    Q_ASSERT(index < frameSetList().size());
    Q_ASSERT(frameSetFile().type == FrameSetType::METASPRITE);

    setFilename(QString::fromStdString(frameSetFile().filename));

    resetState();
}

QStringList ResourceItem::frameNames() const
{
    if (_frameSet) {
        return convertNameList(_frameSet->frames);
    }
    else {
        return QStringList();
    }
}

unsigned ResourceItem::nPalettes() const
{
    if (_frameSet == nullptr) {
        return 1;
    }
    return _frameSet->palettes.size();
}

const UnTech::idstring& ResourceItem::exportOrder() const
{
    static idstring BLANK;

    if (auto* fs = frameSet()) {
        return fs->exportOrder;
    }
    else {
        return BLANK;
    }
}

void ResourceItem::resetState()
{
    if (const MS::FrameSet* fs = frameSet()) {
        setName(QString::fromStdString(fs->name));
    }
    else {
        setName(QString());
    }

    onFrameSetExportOrderChanged();

    paletteList()->setSelectedIndex(0);
    frameList()->unselectItem();
    animationsList()->unselectItem();
}

void ResourceItem::saveResourceData(const std::string& filename) const
{
    MS::saveFrameSet(*_frameSet, filename);
}

bool ResourceItem::loadResourceData(ErrorList& err)
{
    using FrameSetFile = UnTech::MetaSprite::FrameSetFile;

    Q_ASSERT(_frameSet == nullptr);

    FrameSetFile& fsf = frameSetFile();
    Q_ASSERT(fsf.type == FrameSetType::METASPRITE);

    auto oldFrameSet = std::move(fsf.msFrameSet);
    _frameSet = nullptr;
    emit resourceLoaded();

    const std::string& fn = filename().toStdString();
    Q_ASSERT(fn == fsf.filename);

    if (fn.empty()) {
        err.addError("Missing filename");
        return false;
    }

    frameSetFile().loadFile();

    _frameSet = fsf.msFrameSet.get();
    resetState();

    return true;
}

bool ResourceItem::compileResource(ErrorList& err)
{
    compileMsFrameset(_frameSet, err);

    return err.hasError() == false;
}

bool ResourceItem::editFrameSet_setName(const idstring& name)
{
    if (name.isValid() == false) {
        return false;
    }

    return UndoHelper(this).editName(name);
}

bool ResourceItem::editFrameSet_setTilesetType(TilesetType ts)
{
    return UndoHelper(this).editField(
        ts,
        tr("Edit Tileset Type"),
        [](MS::FrameSet& fs) -> TilesetType& { return fs.tilesetType; },
        [](ResourceItem& d) { emit d.frameSetDataChanged(); });
}

bool ResourceItem::editFrameSet_setExportOrder(const UnTech::idstring& exportOrder)
{
    return UndoHelper(this).editField(
        exportOrder,
        tr("Edit Export Order"),
        [](MS::FrameSet& fs) -> idstring& { return fs.exportOrder; },
        [](ResourceItem& d) { emit d.frameSetDataChanged();
                          emit d.frameSetExportOrderChanged(); });
}
