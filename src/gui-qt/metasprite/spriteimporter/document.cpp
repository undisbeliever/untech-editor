/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "accessors.h"
#include "gui-qt/accessor/resourceitemundohelper.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/animation/accessors.h"

using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Document::Document(FrameSetResourceList* parent, size_t index)
    : AbstractMsDocument(parent, index)
    , _frameSet(nullptr)
    , _frameList(new FrameList(this))
    , _frameObjectList(new FrameObjectList(this))
    , _actionPointList(new ActionPointList(this))
    , _entityHitboxList(new EntityHitboxList(this))
{
    Q_ASSERT(index < frameSetList().size());
    Q_ASSERT(frameSetFile().type == FrameSetType::SPRITE_IMPORTER);

    setFilename(QString::fromStdString(frameSetFile().filename));

    resetDocumentState();

    connect(this, &Document::frameSetImageFilenameChanged,
            this, &Document::onFrameSetImageFilenameChanged);
}

QStringList Document::frameNames() const
{
    if (_frameSet) {
        return convertNameList(_frameSet->frames);
    }
    else {
        return QStringList();
    }
}

unsigned Document::nPalettes() const
{
    if (_frameSet == nullptr) {
        return 1;
    }

    unsigned nPalettes = _frameSet->palette.nPalettes;
    if (nPalettes == 0) {
        nPalettes = 1;
    }
    return nPalettes;
}

const UnTech::idstring& Document::exportOrder() const
{
    static idstring BLANK;

    if (auto* fs = frameSet()) {
        return fs->exportOrder;
    }
    else {
        return BLANK;
    }
}

void Document::resetDocumentState()
{
    if (const SI::FrameSet* fs = frameSet()) {
        setName(QString::fromStdString(fs->name));
    }
    else {
        setName(QString());
    }

    onFrameSetExportOrderChanged();
    onFrameSetImageFilenameChanged();

    frameList()->unselectItem();
    animationsList()->unselectItem();
}

void Document::saveResourceData(const std::string& filename) const
{
    SI::saveFrameSet(*_frameSet, filename);
}

bool Document::loadResourceData(ErrorList& err)
{
    using FrameSetFile = UnTech::MetaSprite::FrameSetFile;

    Q_ASSERT(_frameSet == nullptr);

    FrameSetFile& fsf = frameSetFile();
    Q_ASSERT(fsf.type == FrameSetType::SPRITE_IMPORTER);

    auto oldFrameSet = std::move(fsf.siFrameSet);
    _frameSet = nullptr;
    emit resourceLoaded();

    const std::string& fn = filename().toStdString();
    Q_ASSERT(fn == fsf.filename);

    if (fn.empty()) {
        err.addError("Missing filename");
        return false;
    }

    frameSetFile().loadFile();

    _frameSet = fsf.siFrameSet.get();
    resetDocumentState();

    return true;
}

bool Document::compileResource(ErrorList& err)
{
    using FrameSetFile = UnTech::MetaSprite::FrameSetFile;

    FrameSetFile& fsf = frameSetFile();

    fsf.convertSpriteImporter(err);
    compileMsFrameset(fsf.msFrameSet.get(), err);

    return err.hasError() == false;
}

void Document::onFrameSetImageFilenameChanged()
{
    const SI::FrameSet* fs = frameSet();

    QStringList filenames;
    if (fs && fs->imageFilename.empty() == false) {
        filenames << QString::fromStdString(fs->imageFilename);
    }
    setExternalFiles(filenames);
}

bool Document::editFrameSet_setName(const idstring& name)
{
    if (name.isValid() == false) {
        return false;
    }

    return UndoHelper(this).editName(name);
}

bool Document::editFrameSet_setTilesetType(TilesetType ts)
{
    return UndoHelper(this).editField(
        ts,
        tr("Edit Tileset Type"),
        [](SI::FrameSet& fs) -> TilesetType& { return fs.tilesetType; },
        [](Document& d) { emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setExportOrder(const UnTech::idstring& exportOrder)
{
    return UndoHelper(this).editField(
        exportOrder,
        tr("Edit Export Order"),
        [](SI::FrameSet& fs) -> idstring& { return fs.exportOrder; },
        [](Document& d) { emit d.frameSetDataChanged();
                          emit d.frameSetExportOrderChanged(); });
}

bool Document::editFrameSet_setImageFilename(const std::string& filename)
{
    return UndoHelper(this).editField(
        filename,
        tr("Change Image"),
        [](SI::FrameSet& fs) -> std::string& { return fs.imageFilename; },
        [](Document& d) {
            emit d.frameSetImageFilenameChanged();
            emit d.frameSetDataChanged();
        });
}

bool Document::editFrameSet_setTransparentColor(const UnTech::rgba& color)
{
    return UndoHelper(this).editField(
        color,
        tr("Edit Transparent Color"),
        [](SI::FrameSet& fs) -> rgba& { return fs.transparentColor; },
        [](Document& d) { emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setGrid(const SI::FrameSetGrid& grid)
{
    return UndoHelper(this).editField(
        grid,
        tr("Edit FrameSet Grid"),
        [](SI::FrameSet& fs) -> SI::FrameSetGrid& { return fs.grid; },
        [](Document& d) { d.frameSet()->updateFrameLocations();
                          emit d.frameSetGridChanged();
                          emit d.frameSetDataChanged(); });
}

bool Document::editFrameSet_setPalette(const SI::UserSuppliedPalette& palette)
{
    return UndoHelper(this).editField(
        palette,
        tr("Edit FrameSet Palette"),
        [](SI::FrameSet& fs) -> SI::UserSuppliedPalette& { return fs.palette; },
        [](Document& d) { emit d.frameSetPaletteChanged();
                          emit d.frameSetDataChanged(); });
}
