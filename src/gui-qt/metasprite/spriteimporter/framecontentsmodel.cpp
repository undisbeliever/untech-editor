/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framecontentsmodel.h"
#include "document.h"
#include "framecontentcommands.h"
#include "framecontentsdelegate.h"
#include "selection.h"

#include <QHash>
#include <QPoint>
#include <QRect>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

QHash<int, QString> buildEntityHitboxMap()
{
    QHash<int, QString> map;

    for (const auto& it : UnTech::MetaSprite::EntityHitboxType::enumMap) {
        QString s = QString::fromStdString(it.second);
        map.insert(int(it.first), s);
    }
    return map;
}
const QHash<int, QString> FrameContentsModel::entityHitboxMap = buildEntityHitboxMap();

FrameContentsModel::FrameContentsModel(QObject* parent)
    : QAbstractItemModel(parent)
    , _document(nullptr)
    , _frame(nullptr)
{
}

void FrameContentsModel::setDocument(Document* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->selection()->disconnect(this);
    }
    _document = document;

    onSelectedFrameChanged();

    connect(_document->selection(), SIGNAL(selectedFrameChanged()),
            this, SLOT(onSelectedFrameChanged()));
}

void FrameContentsModel::onSelectedFrameChanged()
{
    SI::Frame* frame = _document->selection()->selectedFrame();

    if (_frame != frame) {
        beginResetModel();
        _frame = frame;
        endResetModel();
    }
}

inline SI::FrameObject* FrameContentsModel::toFrameObject(const QModelIndex& index) const
{
    if (_frame == nullptr
        || index.internalId() != FRAME_OBJECT
        || index.row() < 0 || (unsigned)index.row() >= _frame->objects.size()) {
        return nullptr;
    }
    return &_frame->objects.at(index.row());
}

inline SI::ActionPoint* FrameContentsModel::toActionPoint(const QModelIndex& index) const
{
    if (_frame == nullptr
        || index.internalId() != ACTION_POINT
        || index.row() < 0 || (unsigned)index.row() >= _frame->actionPoints.size()) {
        return nullptr;
    }
    return &_frame->actionPoints.at(index.row());
}

inline SI::EntityHitbox* FrameContentsModel::toEntityHitbox(const QModelIndex& index) const
{
    if (_frame == nullptr
        || index.internalId() != ENTITY_HITBOX
        || index.row() < 0 || (unsigned)index.row() >= _frame->entityHitboxes.size()) {
        return nullptr;
    }
    return &_frame->entityHitboxes.at(index.row());
}

QModelIndex FrameContentsModel::index(int row, int column, const QModelIndex& parent) const
{
    if (_frame == nullptr
        || row < 0
        || column < 0 || column > N_COLUMNS) {

        return QModelIndex();
    }

    if (!parent.isValid()) {
        if (row < N_ROOT_NODES) {
            return createIndex(row, column, ROOT);
        }
        return QModelIndex();
    }
    else {
        switch ((InternalId)parent.row() + 1) {
        case FRAME_OBJECT:
            if ((unsigned)row < _frame->objects.size()) {
                return createIndex(row, column, FRAME_OBJECT);
            }
            return QModelIndex();

        case ACTION_POINT:
            if ((unsigned)row < _frame->actionPoints.size()) {
                return createIndex(row, column, ACTION_POINT);
            }
            return QModelIndex();

        case ENTITY_HITBOX:
            if ((unsigned)row < _frame->entityHitboxes.size()) {
                return createIndex(row, column, ENTITY_HITBOX);
            }
            return QModelIndex();

        default:
            return QModelIndex();
        }
    }
}

QModelIndex FrameContentsModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()
        || index.internalId() == ROOT) {

        return QModelIndex();
    }

    if (index.internalId() <= N_ROOT_NODES) {
        return createIndex(index.internalId() - 1, 0, ROOT);
    }
    return QModelIndex();
}

int FrameContentsModel::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid() || parent.internalId() == ROOT) {
        return N_COLUMNS;
    }
    return 0;
}

int FrameContentsModel::rowCount(const QModelIndex& parent) const
{
    if (_frame == nullptr) {
        return 0;
    }

    if (!parent.isValid()) {
        return N_ROOT_NODES;
    }
    else {
        switch ((InternalId)parent.row() + 1) {
        case FRAME_OBJECT:
            return _frame->objects.size();

        case ACTION_POINT:
            return _frame->actionPoints.size();

        case ENTITY_HITBOX:
            return _frame->entityHitboxes.size();

        default:
            return 0;
        }
    }
}

Qt::ItemFlags FrameContentsModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()
        || index.column() >= N_COLUMNS) {
        return 0;
    }

    if (index.internalId() == ROOT) {
        return Qt::ItemIsEnabled;
    }
    else {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
}

QVariant FrameContentsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return QAbstractItemModel::headerData(section, orientation, role);
    }

    switch ((Column)section) {
    case Column::LOCATION:
        return tr("Location");

    case Column::PARAMETER:
        return tr("Parameter");
    }
    return QVariant();
}

QVariant FrameContentsModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid()) {
        switch ((InternalId)index.internalId()) {
        case InternalId::ROOT:
            return data_root(index, role);

        case InternalId::FRAME_OBJECT:
            return data_frameObject(index, role);

        case InternalId::ACTION_POINT:
            return data_actionPoint(index, role);

        case InternalId::ENTITY_HITBOX:
            return data_entityHitbox(index, role);
        }
    }

    return QVariant();
}

QVariant FrameContentsModel::data_root(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole && index.column() == 0) {
        switch ((InternalId)index.row() + 1) {
        case FRAME_OBJECT:
            return tr("Frame Objects");

        case ACTION_POINT:
            return tr("Action Points");

        case ENTITY_HITBOX:
            return tr("Entity Hitboxes");
        }
    }

    return QVariant();
}

QVariant FrameContentsModel::data_frameObject(const QModelIndex& index, int role) const
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    if (const SI::FrameObject* obj = toFrameObject(index)) {
        if (role == Qt::DisplayRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return QString("%1, %2").arg(obj->location.x).arg(obj->location.y);

            case Column::PARAMETER:
                return obj->size == ObjSize::SMALL ? tr("Small") : tr("Large");
            }
        }
        else if (role == Qt::EditRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return QPoint(obj->location.x, obj->location.y);

            case Column::PARAMETER:
                return obj->size != ObjSize::SMALL;
            }
        }
        else if (role == FrameContentsDelegate::DataTypeRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return FrameContentsDelegate::UPOINT;

            case Column::PARAMETER:
                return FrameContentsDelegate::OBJECT_SIZE;
            }
        }
        else if (role == FrameContentsDelegate::RangeRole) {
            const usize s = _frame->location.aabb.size();
            const unsigned o = obj->sizePx();
            return QSize(s.width - o, s.height - o);
        }
    }

    return QVariant();
}

QVariant FrameContentsModel::data_actionPoint(const QModelIndex& index, int role) const
{
    if (const SI::ActionPoint* ap = toActionPoint(index)) {
        if (role == Qt::DisplayRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return QString("%1, %2").arg(ap->location.x).arg(ap->location.y);

            case Column::PARAMETER:
                return int(ap->parameter);
            }
        }
        else if (role == Qt::EditRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return QPoint(ap->location.x, ap->location.y);

            case Column::PARAMETER:
                return unsigned(ap->parameter);
            }
        }
        else if (role == FrameContentsDelegate::DataTypeRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return FrameContentsDelegate::UPOINT;

            case Column::PARAMETER:
                return FrameContentsDelegate::ACTION_POINT_PARAMETER;
            }
        }
        else if (role == FrameContentsDelegate::RangeRole) {
            const usize s = _frame->location.aabb.size();
            return QSize(s.width, s.height);
        }
    }

    return QVariant();
}

QVariant FrameContentsModel::data_entityHitbox(const QModelIndex& index, int role) const
{
    if (const SI::EntityHitbox* eh = toEntityHitbox(index)) {
        if (role == Qt::DisplayRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return QString("%1, %2 : %3 x %4")
                    .arg(eh->aabb.x)
                    .arg(eh->aabb.y)
                    .arg(eh->aabb.width)
                    .arg(eh->aabb.height);

            case Column::PARAMETER:
                return entityHitboxMap.value(int(eh->hitboxType.value()));
            }
        }
        else if (role == Qt::EditRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return QRect(eh->aabb.x, eh->aabb.y,
                             eh->aabb.width, eh->aabb.height);

            case Column::PARAMETER:
                return int(eh->hitboxType.value());
            }
        }
        else if (role == FrameContentsDelegate::DataTypeRole) {
            switch ((Column)index.column()) {
            case Column::LOCATION:
                return FrameContentsDelegate::URECT;

            case Column::PARAMETER:
                return FrameContentsDelegate::ENTITY_HITBOX_TYPE;
            }
        }
        else if (role == FrameContentsDelegate::RangeRole) {
            const usize s = _frame->location.aabb.size();
            return QSize(s.width, s.height);
        }
    }

    return QVariant();
}

bool FrameContentsModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (_frame == nullptr
        || role != Qt::EditRole) {
        return false;
    }

    switch ((InternalId)index.internalId()) {
    case InternalId::FRAME_OBJECT:
        return setData_frameObject(index, value);

    case InternalId::ACTION_POINT:
        return setData_actionPoint(index, value);

    case InternalId::ENTITY_HITBOX:
        return setData_entityHitbox(index, value);

    default:
        return false;
    }
}

bool FrameContentsModel::setData_frameObject(const QModelIndex& index, const QVariant& value)
{
    using ObjSize = UnTech::MetaSprite::ObjectSize;

    const SI::FrameObject* modelObj = toFrameObject(index);
    if (modelObj == nullptr) {
        return false;
    }

    SI::FrameObject obj = *modelObj;

    switch ((Column)index.column()) {
    case Column::LOCATION:
        obj.location.x = value.toPoint().x();
        obj.location.y = value.toPoint().y();
        break;

    case Column::PARAMETER:
        obj.size = value.toBool() ? ObjSize::LARGE : ObjSize::SMALL;
        break;

    default:
        return false;
    }

    if (obj != *modelObj && obj.isValid(_frame->location)) {
        _document->undoStack()->push(
            new ChangeFrameObject(_document, _frame, index.row(), obj));
    }

    return true;
}

bool FrameContentsModel::setData_actionPoint(const QModelIndex& index, const QVariant& value)
{
    const SI::ActionPoint* modelAp = toActionPoint(index);
    if (modelAp == nullptr) {
        return false;
    }

    SI::ActionPoint ap = *modelAp;

    switch ((Column)index.column()) {
    case Column::LOCATION:
        ap.location.x = value.toPoint().x();
        ap.location.y = value.toPoint().y();
        break;

    case Column::PARAMETER:
        ap.parameter = value.toUInt();
        break;

    default:
        return false;
    }

    if (ap != *modelAp && ap.isValid(_frame->location)) {
        _document->undoStack()->push(
            new ChangeActionPoint(_document, _frame, index.row(), ap));
    }

    return true;
}

bool FrameContentsModel::setData_entityHitbox(const QModelIndex& index, const QVariant& value)
{
    using EntityHitboxType = UnTech::MetaSprite::EntityHitboxType;

    const SI::EntityHitbox* modelEh = toEntityHitbox(index);
    if (modelEh == nullptr) {
        return false;
    }

    SI::EntityHitbox eh = *modelEh;

    switch ((Column)index.column()) {
    case Column::LOCATION:
        eh.aabb.x = value.toRect().x();
        eh.aabb.y = value.toRect().y();
        eh.aabb.width = value.toRect().width();
        eh.aabb.height = value.toRect().height();
        break;

    case Column::PARAMETER:
        eh.hitboxType = (EntityHitboxType::Enum)value.toInt();
        break;

    default:
        return false;
    }

    if (eh != *modelEh && eh.isValid(_frame->location)) {
        _document->undoStack()->push(
            new ChangeEntityHitbox(_document, _frame, index.row(), eh));
    }

    return true;
}

#define SET_ITEM(CLS, FIELD, INTERNAL_ID, SIGNAL)                           \
    void FrameContentsModel::set##CLS(SI::Frame* frame,                     \
                                      unsigned index, const SI::CLS& value) \
    {                                                                       \
        frame->FIELD.at(index) = value;                                     \
                                                                            \
        if (_frame == frame) {                                              \
            emit dataChanged(createIndex(index, 0, INTERNAL_ID),            \
                             createIndex(index, N_COLUMNS, INTERNAL_ID),    \
                             { Qt::DisplayRole, Qt::EditRole });            \
        }                                                                   \
        emit _document->SIGNAL(frame, index);                               \
    }
SET_ITEM(FrameObject, objects, FRAME_OBJECT, frameObjectChanged);
SET_ITEM(ActionPoint, actionPoints, ACTION_POINT, actionPointChanged);
SET_ITEM(EntityHitbox, entityHitboxes, ENTITY_HITBOX, entityHitboxChanged);
