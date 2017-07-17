/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "framelistmodel.h"
#include "selection.h"
#include "gui-qt/metasprite/abstractdocument.h"
#include "models/metasprite/metasprite.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class FrameContentsModel;
class PalettesModel;

namespace MS = UnTech::MetaSprite::MetaSprite;

class Document : public AbstractDocument {
    Q_OBJECT

public:
    static const char* FILE_FILTER;

public:
    explicit Document(QObject* parent = nullptr);
    Document(std::unique_ptr<MS::FrameSet> frameSet,
             const QString& filename, QObject* parent = nullptr);
    ~Document() = default;

    static std::unique_ptr<Document> loadDocument(const QString& filename);
    virtual bool saveDocument(const QString& filename) final;

    MS::FrameSet* frameSet() const { return _frameSet.get(); }
    virtual MSA::Animation::map_t* animations() const final { return &_frameSet->animations; }

    virtual Selection* selection() const final { return _selection; }
    virtual FrameListModel* frameListModel() const final { return _frameListModel; }
    FrameContentsModel* frameContentsModel() const { return _frameContentsModel; }
    PalettesModel* palettesModel() const { return _palettesModel; }

    virtual AbstractSelection* abstractSelection() const { return nullptr; }

signals:
    void paletteChanged(unsigned index);
    void paletteAdded(unsigned index);
    void paletteAboutToBeRemoved(unsigned index);
    void paletteMoved(unsigned oldIndex, unsigned newIndex);

private:
    std::unique_ptr<MS::FrameSet> _frameSet;

    Selection* _selection;
    FrameListModel* _frameListModel;
    FrameContentsModel* _frameContentsModel;
    PalettesModel* _palettesModel;
};
}
}
}
}
