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

    connect(this, &Document::frameSetNameChanged,
            this, &Document::onFrameSetNameChanged);
}

QStringList Document::frameList() const
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
    if (_frameSet) {
        setName(QString::fromStdString(_frameSet->name));
    }
    else {
        setName(QString());
    }

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

void Document::onFrameSetNameChanged()
{
    const MS::FrameSet* fs = frameSet();

    if (fs) {
        setName(QString::fromStdString(fs->name));
    }
    else {
        setName(QString());
    }
}
