/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include <QAbstractListModel>
#include <QStringList>
#include <QVector>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

class FrameListModel : public QAbstractListModel {
    Q_OBJECT

public:
    explicit FrameListModel(QObject* parent = nullptr);
    ~FrameListModel() = default;

    void setDocument(Document* document);

    QModelIndex toModelIndex(const idstring& frameId) const;
    idstring toFrameId(int row) const;
    idstring toFrameId(const QModelIndex& index) const;

    virtual int rowCount(const QModelIndex& parent) const final;
    virtual QVariant data(const QModelIndex& index, int role) const final;

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
