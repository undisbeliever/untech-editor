/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourceitem.h"
#include "models/metatiles/metatiles-serializer.h"

using namespace UnTech::GuiQt::Resources;

MtTilesetResourceItem::MtTilesetResourceItem(AbstractResourceList* parent, size_t index)
    : AbstractExternalResourceItem(parent, index)
{
    Q_ASSERT(index < mtTilesetFilenameList().size());

    setFilename(QString::fromStdString(mtTilesetFilename()));
}

QString MtTilesetResourceItem::name() const
{
    if (_tilesetInput) {
        return QString::fromStdString(_tilesetInput->name);
    }
    else {
        return relativeFilePath();
    }
}

bool MtTilesetResourceItem::loadResourceData(RES::ErrorList& err)
{
    const std::string& fn = mtTilesetFilename();

    if (fn.empty()) {
        err.addError("Missing filename");
        return false;
    }

    try {
        _tilesetInput = MT::loadMetaTileTilesetInput(fn);
        return true;
    }
    catch (const std::exception& ex) {
        _tilesetInput = nullptr;

        err.addError(ex.what());
        return false;
    }
}

bool MtTilesetResourceItem::compileResource(RES::ErrorList& err)
{
    if (_tilesetInput == nullptr) {
        err.addError("Unable to load file");
        return false;
    }
    const auto& res = _document->resourcesFile();
    Q_ASSERT(res);

    const auto mtd = MetaTiles::convertTileset(*_tilesetInput, *res, err);
    return mtd && mtd->validate(res->metaTileEngineSettings, err);
}
