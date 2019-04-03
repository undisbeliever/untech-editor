/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/accessor/accessor.h"
#include "models/common/namedlist.h"
#include "models/resources/palette.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {
namespace Palette {
class ResourceList;

namespace RES = UnTech::Resources;

class ResourceItem : public AbstractInternalResourceItem {
    Q_OBJECT

    using UndoHelper = Accessor::ResourceItemUndoHelper<ResourceItem>;

public:
    using DataT = RES::PaletteInput;

public:
    ResourceItem(ResourceList* parent, size_t index);
    ~ResourceItem() = default;

    static QString typeName() { return tr("Palette"); }

    inline const RES::PaletteInput& paletteInput() const { return _palettes.at(index()); }

    // returns nullptr if the PaletteData is invalid
    const RES::PaletteData* compiledData() const { return _compiledData.get(); }

    bool editPalette_setName(const idstring& name);
    bool editPalette_setImageFilename(const std::string& filename);
    bool editPalette_setRowsPerFrame(unsigned rowsPerFrame);
    bool editPalette_setAnimationDelay(unsigned animationDelay);
    bool editPalette_setSkipFirstFrame(bool skipFirstFrame);

private:
    friend class Accessor::ResourceItemUndoHelper<ResourceItem>;
    const RES::PaletteInput* data() const { return &_palettes.at(index()); }
    RES::PaletteInput* dataEditable() { return &_palettes.at(index()); }

    void updateExternalFiles();

protected:
    virtual bool compileResource(ErrorList& err) final;

private:
    NamedList<RES::PaletteInput>& _palettes;

    std::unique_ptr<RES::PaletteData> _compiledData;
};
}
}
}
}
