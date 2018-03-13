/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framelistmodel.h"
#include "selection.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include "models/metasprite/metasprite.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class PalettesModel;

namespace MS = UnTech::MetaSprite::MetaSprite;

class Document : public AbstractMsDocument {
    Q_OBJECT

public:
    using FrameT = MS::Frame;

public:
    explicit Document(QObject* parent = nullptr);
    ~Document() = default;

    MS::FrameSet* frameSet() const { return _frameSet.get(); }
    virtual MSA::Animation::map_t* animations() const final { return &_frameSet->animations; }

    virtual Selection* selection() const final { return _selection; }
    virtual FrameListModel* frameListModel() const final { return _frameListModel; }
    PalettesModel* palettesModel() const { return _palettesModel; }

    virtual const QString& fileFilter() const final;
    virtual const QString& defaultFileExtension() const final;

protected:
    virtual bool saveDocumentFile(const QString& filename) final;
    virtual bool loadDocumentFile(const QString& filename) final;

private:
    void initModels();

signals:
    void paletteChanged(unsigned index);
    void paletteAdded(unsigned index);
    void paletteAboutToBeRemoved(unsigned index);
    void paletteMoved(unsigned oldIndex, unsigned newIndex);

    void smallTilesetChanged();
    void largeTilesetChanged();

    void smallTileChanged(unsigned tileId);
    void largeTileChanged(unsigned tileId);

private:
    std::unique_ptr<MS::FrameSet> _frameSet;

    Selection* _selection;
    FrameListModel* _frameListModel;
    PalettesModel* _palettesModel;
};
}
}
}
}
