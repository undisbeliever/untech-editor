/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/spriteimporter.h"
#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;
class AddRemoveFrame;
class RenameFrame;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class FrameListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit FrameListModel(QObject* parent = nullptr);
    ~FrameListModel() = default;

    void setDocument(Document* document);

    QModelIndex toModelIndex(const idstring& frameId) const;
    idstring toFrameId(int row) const;
    idstring toFrameId(const QModelIndex& index) const;

    const QStringList& frameNames() const { return _frameNames; }

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual QVariant data(const QModelIndex& index, int role) const final;

protected:
    friend class AddRemoveFrame;
    void insertFrame(const idstring& id, std::unique_ptr<SI::Frame> frame);
    std::unique_ptr<SI::Frame> removeFrame(const idstring& id);

    friend class RenameFrame;
    void renameFrame(const idstring& oldId, const idstring& newId);

private:
    void buildFrameLists();

private:
    Document* _document;
    QStringList _frameNames;
    QVector<idstring> _frameIdstrings;
};
}
}
}
}
