/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "accessors.h"
#include "gui-qt/metasprite/animation/animationaccessors.h"

using FrameSetType = UnTech::MetaSprite::Project::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Document::Document(FrameSetResourceList* parent, size_t index)
    : AbstractMsDocument(parent, index)
    , _frameSet(nullptr)
    , _smallTileTileset(new SmallTileTileset(this))
    , _largeTileTileset(new LargeTileTileset(this))
    , _paletteList(new PaletteList(this))
    , _frameMap(new FrameMap(this))
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
    QStringList fl;

    if (_frameSet) {
        auto& frames = _frameSet->frames;

        fl.reserve(frames.size());
        for (const auto& it : frames) {
            fl.append(QString::fromStdString(it.first));
        }
    }

    return fl;
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
    frameMap()->unselectItem();
    animationsMap()->unselectItem();
}

void Document::saveResourceData(const std::string& filename) const
{
    MS::saveFrameSet(*_frameSet, filename);
}

bool Document::loadResourceData(RES::ErrorList& err)
{
    using FrameSetFile = UnTech::MetaSprite::Project::FrameSetFile;

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

bool Document::compileResource(RES::ErrorList& err)
{
    UnTech::MetaSprite::ErrorList msErrorList;

    compileMsFrameset(_frameSet, msErrorList);
    appendToErrorList(err, msErrorList);

    return msErrorList.errors.empty() == true;
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
