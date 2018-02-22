/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourceitem.h"
#include "models/metatiles/metatiles-serializer.h"

#include <QDir>

using namespace UnTech::GuiQt::Resources;

MtTilesetResourceItem::MtTilesetResourceItem(AbstractResourceList* parent, size_t index)
    : AbstractResourceItem(parent, index)
{
    Q_ASSERT(index < mtTilesetFilenameList().size());

    updateFilePaths();

    connect(document(), &Document::filenameChanged,
            this, &MtTilesetResourceItem::updateFilePaths);
}

void MtTilesetResourceItem::updateFilePaths()
{
    QString mf = QString::fromStdString(mtTilesetFilename());

    QDir dir(document()->filename());
    _absoluteFilePath = dir.absolutePath();

    bool s = dir.cdUp();
    if (s) {
        _relativeFilePath = dir.relativeFilePath(mf);
    }
    else {
        _relativeFilePath = _absoluteFilePath;
    }
}

void MtTilesetResourceItem::loadPixmaps()
{
    _pixmaps.clear();

    if (_tilesetInput) {
        const auto& filenames = _tilesetInput->animationFrames.frameImageFilenames;

        _pixmaps.reserve(filenames.size());
        for (const auto& fn : filenames) {
            _pixmaps.append(QPixmap(QString::fromStdString(fn), "PNG"));
        }
    }
}

void MtTilesetResourceItem::unloadPixmaps()
{
    _pixmaps.clear();
}

const QString MtTilesetResourceItem::name() const
{
    if (_tilesetInput) {
        return QString::fromStdString(_tilesetInput->name);
    }
    else {
        return _relativeFilePath;
    }
}

const QString MtTilesetResourceItem::filename() const
{
    return _absoluteFilePath;
}

bool MtTilesetResourceItem::loadResourceData(RES::ErrorList& err)
{
    const auto& fn = mtTilesetFilename();

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
