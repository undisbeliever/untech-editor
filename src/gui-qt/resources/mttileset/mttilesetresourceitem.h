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
#include <QPixmap>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace Resources {

namespace MT = UnTech::MetaTiles;

class MtTilesetResourceItem : public AbstractResourceItem {
    Q_OBJECT

public:
    MtTilesetResourceItem(AbstractResourceList* parent, size_t index);
    ~MtTilesetResourceItem() = default;

public:
    virtual const QString name() const final;
    virtual const QString filename() const final;

protected:
    virtual bool compileResource(RES::ErrorList& err) final;

public:
    // may be nullptr
    const auto& tilesetInput() const { return _tilesetInput; }

    // may be empty or invalid
    const auto& pixmaps() const { return _pixmaps; }

public:
    void loadPixmaps();
    void unloadPixmaps();

private:
    inline const auto& mtTilesetFilenameList() const
    {
        return document()->resourcesFile()->metaTileTilesetFilenames;
    }

    inline const std::string& mtTilesetFilename() const
    {
        return document()->resourcesFile()->metaTileTilesetFilenames.at(index());
    }

    void loadFile();

private slots:
    void updateFilePaths();

private:
    QString _absoluteFilePath;
    QString _relativeFilePath;

    std::unique_ptr<MT::MetaTileTilesetInput> _tilesetInput;
    QVector<QPixmap> _pixmaps;
};
}
}
}
