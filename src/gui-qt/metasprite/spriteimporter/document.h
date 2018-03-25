/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framelistmodel.h"
#include "selection.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "models/metasprite/spriteimporter.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class FrameContentsModel;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class Document : public AbstractMsDocument {
    Q_OBJECT

public:
    using FrameT = SI::Frame;

public:
    Document(FrameSetResourceList* parent, size_t index);
    ~Document() = default;

    SI::FrameSet* frameSet() const { return _frameSet.get(); }
    virtual MSA::Animation::map_t* animations() const final { return &_frameSet->animations; }

    virtual Selection* selection() const final { return _selection; }
    virtual FrameListModel* frameListModel() const final { return _frameListModel; }

protected:
    // can throw exceptions
    virtual void saveResourceData(const std::string& filename) const final;
    virtual bool loadResourceData(RES::ErrorList& err) final;
    virtual bool compileResource(RES::ErrorList& err) final;

private:
    void initModels();

signals:
    void frameSetGridChanged();
    void frameSetImageFilenameChanged();
    void frameSetImageChanged();
    void frameSetPaletteChanged();

    void frameLocationChanged(const void* frame);

private:
    std::unique_ptr<SI::FrameSet> _frameSet;

    Selection* _selection;
    FrameListModel* _frameListModel;
};
}
}
}
}
