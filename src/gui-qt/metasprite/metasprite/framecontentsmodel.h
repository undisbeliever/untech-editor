/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "selection.h"
#include "models/metasprite/metasprite.h"
#include <QAbstractItemModel>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace MetaSprite {
class Document;
class ChangeFrameObject;
class ChangeActionPoint;
class ChangeEntityHitbox;
class AddRemoveFrameObject;
class AddRemoveActionPoint;
class AddRemoveEntityHitbox;
class AddRemoveSmallTile;
class AddRemoveLargeTile;

namespace MS = UnTech::MetaSprite::MetaSprite;

class FrameContentsModel : public QAbstractItemModel {
    Q_OBJECT

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
    MS::FrameObject* toFrameObject(const QModelIndex& index) const;
    MS::ActionPoint* toActionPoint(const QModelIndex& index) const;
    MS::EntityHitbox* toEntityHitbox(const QModelIndex& index) const;

    QVariant data_root(const QModelIndex& index, int role) const;
    QVariant data_frameObject(const QModelIndex& index, int role) const;
    QVariant data_actionPoint(const QModelIndex& index, int role) const;
    QVariant data_entityHitbox(const QModelIndex& index, int role) const;

    bool setData_frameObject(const QModelIndex& index, const QVariant& value);
    bool setData_actionPoint(const QModelIndex& index, const QVariant& value);
    bool setData_entityHitbox(const QModelIndex& index, const QVariant& value);

protected:
    friend class ChangeFrameObject;
    friend class AddRemoveSmallTile;
    friend class AddRemoveLargeTile;
    void setFrameObject(MS::Frame* frame, unsigned index, const MS::FrameObject& obj);

    friend class ChangeActionPoint;
    void setActionPoint(MS::Frame* frame, unsigned index, const MS::ActionPoint& ap);

    friend class ChangeEntityHitbox;
    void setEntityHitbox(MS::Frame* frame, unsigned index, const MS::EntityHitbox& eh);

    friend class AddRemoveFrameObject;
    void insertFrameObject(MS::Frame* frame, unsigned index, const MS::FrameObject&);
    void removeFrameObject(MS::Frame* frame, unsigned index);

    friend class AddRemoveActionPoint;
    void insertActionPoint(MS::Frame* frame, unsigned index, const MS::ActionPoint&);
    void removeActionPoint(MS::Frame* frame, unsigned index);

    friend class AddRemoveEntityHitbox;
    void insertEntityHitbox(MS::Frame* frame, unsigned index, const MS::EntityHitbox&);
    void removeEntityHitbox(MS::Frame* frame, unsigned index);

    void raiseSelectedItems(MS::Frame* frame, const std::set<SelectedItem>&);
    void lowerSelectedItems(MS::Frame* frame, const std::set<SelectedItem>&);

private slots:
    void onSelectedFrameChanged();

private:
    Document* _document;
    MS::Frame* _frame;
};
}
}
}
}
