/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractidmaplistmodel.h"
#include "models/metasprite/metasprite.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;
class AddRemoveFrame;
class RenameFrame;

namespace MS = UnTech::MetaSprite::MetaSprite;

class FrameListModel : public AbstractIdmapListModel {
    Q_OBJECT

public:
    explicit FrameListModel(QObject* parent = nullptr);
    ~FrameListModel() = default;

    void setDocument(Document* document);

protected:
    friend class AddRemoveFrame;
    void insertFrame(const idstring& id, std::unique_ptr<MS::Frame> frame);
    std::unique_ptr<MS::Frame> removeFrame(const idstring& id);

    friend class RenameFrame;
    void renameFrame(const idstring& oldId, const idstring& newId);

private:
    Document* _document;
};
}
}
}
}
