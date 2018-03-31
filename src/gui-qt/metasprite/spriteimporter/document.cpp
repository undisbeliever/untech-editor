/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "framelistmodel.h"
#include "selection.h"

using FrameSetType = UnTech::MetaSprite::Project::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Document::Document(FrameSetResourceList* parent, size_t index)
    : AbstractMsDocument(parent, index)
    , _frameSet(nullptr)
    , _selection(new Selection(this))
{
    Q_ASSERT(index < frameSetList().size());
    Q_ASSERT(frameSetFile().type == FrameSetType::SPRITE_IMPORTER);

    setFilename(QString::fromStdString(frameSetFile().filename));

    resetDocumentState();
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

    _selection->unselectAll();
}

void Document::saveResourceData(const std::string& filename) const
{
    SI::saveFrameSet(*_frameSet, filename);
}

bool Document::loadResourceData(RES::ErrorList& err)
{
    using FrameSetFile = UnTech::MetaSprite::Project::FrameSetFile;

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

bool Document::compileResource(UnTech::Resources::ErrorList& err)
{
    err.addError("compileResource: Not implemented");
    return false;
}
