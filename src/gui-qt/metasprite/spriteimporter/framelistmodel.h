/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractidmaplistmodel.h"
#include "models/metasprite/spriteimporter.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

class FrameListModel : public AbstractIdmapListModel {
    Q_OBJECT

public:
    explicit FrameListModel(QObject* parent = nullptr);
    ~FrameListModel() = default;

    void setDocument(Document* document);

private:
    Document* _document;
};
}
}
}
}
