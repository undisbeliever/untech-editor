/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "accessors.h"
#include "gui-qt/metasprite/animation/animationaccessors.h"

using FrameSetType = UnTech::MetaSprite::Project::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Document::Document(FrameSetResourceList* parent, size_t index)
    : AbstractMsDocument(parent, index)
    , _frameSet(nullptr)
    , _frameMap(new FrameMap(this))
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
    if (const SI::FrameSet* fs = frameSet()) {
        setName(QString::fromStdString(fs->name));
    }
    else {
        setName(QString());
    }

    onFrameSetExportOrderChanged();
    onFrameSetImageFilenameChanged();

    frameMap()->unselectItem();
    animationsMap()->unselectItem();
}

void Document::saveResourceData(const std::string& filename) const
{
    SI::saveFrameSet(*_frameSet, filename);
}

bool Document::loadResourceData(RES::ErrorList& err)
{
    using FrameSetFile = UnTech::MetaSprite::Project::FrameSetFile;

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

bool Document::compileResource(RES::ErrorList& err)
{
    using FrameSetFile = UnTech::MetaSprite::Project::FrameSetFile;

    FrameSetFile& fsf = frameSetFile();
    UnTech::MetaSprite::ErrorList msErrorList;

    fsf.convertSpriteImporter(msErrorList);
    compileMsFrameset(fsf.msFrameSet.get(), msErrorList);
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

void Document::onFrameSetImageFilenameChanged()
{
    const SI::FrameSet* fs = frameSet();

    QStringList filenames;
    if (fs && fs->imageFilename.empty() == false) {
        filenames << QString::fromStdString(fs->imageFilename);
    }
    setExternalFiles(filenames);
}
