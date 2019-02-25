/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "accessors.h"
#include "gui-qt/common/helpers.h"
#include "gui-qt/metasprite/animation/animationaccessors.h"

using FrameSetType = UnTech::MetaSprite::FrameSetFile::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Document::Document(FrameSetResourceList* parent, size_t index)
    : AbstractMsDocument(parent, index)
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

    resetDocumentState();

    connect(this, &Document::frameSetExportOrderChanged,
            this, &Document::onFrameSetExportOrderChanged);
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
    return _frameSet->palettes.size();
}

void Document::resetDocumentState()
{
    if (const MS::FrameSet* fs = frameSet()) {
        setName(QString::fromStdString(fs->name));
    }
    else {
        setName(QString());
    }

    onFrameSetExportOrderChanged();

    paletteList()->unselectItem();
    frameList()->unselectItem();
    animationsList()->unselectItem();
}

void Document::saveResourceData(const std::string& filename) const
{
    MS::saveFrameSet(*_frameSet, filename);
}

bool Document::loadResourceData(ErrorList& err)
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
    resetDocumentState();

    return true;
}

bool Document::compileResource(ErrorList& err)
{
    compileMsFrameset(_frameSet, err);

    return err.hasError() == false;
}

void Document::onFrameSetExportOrderChanged()
{
    if (auto* fs = frameSet()) {
        setDependencies({
            { ResourceTypeIndex::MS_EXPORT_ORDER, QString::fromStdString(fs->exportOrder) },
        });
    }
    else {
        removeDependencies();
    }
}
