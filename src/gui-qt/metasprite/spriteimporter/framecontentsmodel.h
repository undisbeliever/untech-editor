/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/spriteimporter.h"
#include <QAbstractItemModel>
#include <QHash>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;

namespace SI = UnTech::MetaSprite::SpriteImporter;

class FrameContentsModel : public QAbstractItemModel {
    Q_OBJECT

    const static QHash<int, QString> entityHitboxMap;

public:
    enum class Column {
        LOCATION,
        PARAMETER
    };
    constexpr static int N_COLUMNS = 2;

    enum InternalId {
        ROOT = 0,
        FRAME_OBJECT = 1,
        ACTION_POINT = 2,
        ENTITY_HITBOX = 3
    };
    constexpr static int N_ROOT_NODES = 3;

public:
    explicit FrameContentsModel(QObject* parent = nullptr);
    ~FrameContentsModel() = default;

    void setDocument(Document* document);

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const final;

    virtual QVariant data(const QModelIndex& index, int role) const final;

private:
    SI::FrameObject* toFrameObject(const QModelIndex& index) const;
    SI::ActionPoint* toActionPoint(const QModelIndex& index) const;
    SI::EntityHitbox* toEntityHitbox(const QModelIndex& index) const;

    QVariant data_root(const QModelIndex& index, int role) const;
    QVariant data_frameObject(const QModelIndex& index, int role) const;
    QVariant data_actionPoint(const QModelIndex& index, int role) const;
    QVariant data_entityHitbox(const QModelIndex& index, int role) const;

private slots:
    void onSelectedFrameChanged();

private:
    Document* _document;
    SI::Frame* _frame;
};
}
}
}
}
