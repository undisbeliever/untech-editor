/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "../abstractresourceitem.h"
#include "../document.h"
#include "models/metatiles/metatile-tileset.h"
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace Resources {

namespace MT = UnTech::MetaTiles;

class MtTilesetResourceItem : public AbstractExternalResourceItem {
    Q_OBJECT

public:
    MtTilesetResourceItem(AbstractResourceList* parent, size_t index);
    ~MtTilesetResourceItem() = default;

public:
    virtual QString name() const final;

protected:
    virtual bool loadResourceData(RES::ErrorList& err) final;
    virtual bool compileResource(RES::ErrorList& err) final;

public:
    // may be nullptr
    const auto& tilesetInput() const { return _tilesetInput; }


private:
    inline const auto& mtTilesetFilenameList() const
    {
        return document()->resourcesFile()->metaTileTilesetFilenames;
    }

    inline const std::string& mtTilesetFilename() const
    {
        return document()->resourcesFile()->metaTileTilesetFilenames.at(index());
    }

private:
    std::unique_ptr<MT::MetaTileTilesetInput> _tilesetInput;
};
}
}
}
