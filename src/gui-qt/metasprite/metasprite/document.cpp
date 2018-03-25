/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "framelistmodel.h"
#include "palettesmodel.h"
#include "selection.h"

using FrameSetType = UnTech::MetaSprite::Project::FrameSetType;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Document::Document(FrameSetResourceList* parent, size_t index)
    : AbstractMsDocument(parent, index)
    , _frameSet(std::make_unique<MS::FrameSet>())
    , _selection(new Selection(this))
    , _frameListModel(new FrameListModel(this))
    , _palettesModel(new PalettesModel(this))
{
    Q_ASSERT(index < frameSetList().size());
    Q_ASSERT(frameSetFile().type == FrameSetType::METASPRITE);

    setFilename(QString::fromStdString(frameSetFile().filename));

    initModels();
}

void Document::initModels()
{
    setName(QString::fromStdString(_frameSet->name));

    _selection->setDocument(this);
    _frameListModel->setDocument(this);
    _palettesModel->setDocument(this);

    AbstractMsDocument::initModels();
}

void Document::saveResourceData(const std::string& filename) const
{
    MS::saveFrameSet(*_frameSet, filename);
}

bool Document::loadResourceData(RES::ErrorList& err)
{
    Q_ASSERT(frameSetFile().type == FrameSetType::METASPRITE);

    const std::string& fn = filename().toStdString();

    if (fn.empty()) {
        err.addError("Missing filename");
        return false;
    }

    auto fs = MS::loadFrameSet(fn);
    if (fs) {
        _frameSet = std::move(fs);
        initModels();
        return true;
    }
    return false;
}

bool Document::compileResource(UnTech::Resources::ErrorList& err)
{
    err.addError("compileResource: Not implemented");
    return false;
}
