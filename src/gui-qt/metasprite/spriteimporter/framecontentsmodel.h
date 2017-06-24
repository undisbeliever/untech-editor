/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "selection.h"
#include "models/metasprite/spriteimporter.h"
#include <QAbstractItemModel>
#include <QHash>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace SpriteImporter {
class Document;
class ChangeFrameObject;
class ChangeActionPoint;
class ChangeEntityHitbox;
class AddRemoveFrameObject;
class AddRemoveActionPoint;
class AddRemoveEntityHitbox;

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

    QModelIndex toModelIndex(const SelectedItem& item) const;
    SelectedItem toSelectedItem(const QModelIndex& index) const;

    virtual QModelIndex index(int row, int column, const QModelIndex& parent) const final;
    virtual QModelIndex parent(const QModelIndex& index) const final;

    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const final;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const final;

    virtual Qt::ItemFlags flags(const QModelIndex& index) const final;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const final;

    virtual QVariant data(const QModelIndex& index, int role) const final;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) final;

private:
    SI::FrameObject* toFrameObject(const QModelIndex& index) const;
    SI::ActionPoint* toActionPoint(const QModelIndex& index) const;
    SI::EntityHitbox* toEntityHitbox(const QModelIndex& index) const;

    QVariant data_root(const QModelIndex& index, int role) const;
    QVariant data_frameObject(const QModelIndex& index, int role) const;
    QVariant data_actionPoint(const QModelIndex& index, int role) const;
    QVariant data_entityHitbox(const QModelIndex& index, int role) const;

    bool setData_frameObject(const QModelIndex& index, const QVariant& value);
    bool setData_actionPoint(const QModelIndex& index, const QVariant& value);
    bool setData_entityHitbox(const QModelIndex& index, const QVariant& value);

protected:
    friend class ChangeFrameObject;
    void setFrameObject(SI::Frame* frame, unsigned index, const SI::FrameObject& obj);

    friend class ChangeActionPoint;
    void setActionPoint(SI::Frame* frame, unsigned index, const SI::ActionPoint& ap);

    friend class ChangeEntityHitbox;
    void setEntityHitbox(SI::Frame* frame, unsigned index, const SI::EntityHitbox& eh);

    friend class AddRemoveFrameObject;
    void insertFrameObject(SI::Frame* frame, unsigned index, const SI::FrameObject&);
    void removeFrameObject(SI::Frame* frame, unsigned index);

    friend class AddRemoveActionPoint;
    void insertActionPoint(SI::Frame* frame, unsigned index, const SI::ActionPoint&);
    void removeActionPoint(SI::Frame* frame, unsigned index);

    friend class AddRemoveEntityHitbox;
    void insertEntityHitbox(SI::Frame* frame, unsigned index, const SI::EntityHitbox&);
    void removeEntityHitbox(SI::Frame* frame, unsigned index);

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
