/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/common.h"
#include <QDockWidget>
#include <QList>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
namespace Ui {
class TilesetDock;
}
class Document;
class TilesetPixmaps;

class TilesetDock : public QDockWidget {
    Q_OBJECT

    using ObjectSize = UnTech::MetaSprite::ObjectSize;

public:
    TilesetDock(TilesetPixmaps* tilesetPixmaps, QWidget* parent = nullptr);
    ~TilesetDock();

    void setDocument(Document* document);

private slots:
    void onSelectedFrameObjectsChanged();
    void onFrameObjectChanged(const void* frame, unsigned index);
    void onTileClicked(ObjectSize size, int tileIndex);
    void onContextMenu(const QPoint& pos);

private:
    // returns -1 if more than one frame object is selected
    int selectedFrameObjectIndex() const;

private:
    std::unique_ptr<Ui::TilesetDock> const _ui;

    Document* _document;
};
}
}
}
}
