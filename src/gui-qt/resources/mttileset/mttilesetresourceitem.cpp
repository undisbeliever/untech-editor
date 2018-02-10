/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "mttilesetresourceitem.h"

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

const QString MtTilesetResourceItem::name() const
{
    return _relativeFilePath;
}

const QString MtTilesetResourceItem::filename() const
{
    return _absoluteFilePath;
}
