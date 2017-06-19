/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framelistmodel.h"
#include "document.h"

using namespace UnTech;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameListModel::FrameListModel(QObject* parent)
    : QAbstractListModel(parent)
    , _document(nullptr)
    , _frameNames()
    , _frameIdstrings()
{
}

void FrameListModel::setDocument(Document* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    buildFrameLists();
}

void FrameListModel::buildFrameLists()
{
    beginResetModel();

    _frameNames.clear();
    _frameIdstrings.clear();

    for (const auto& it : _document->frameSet()->frames) {
        _frameNames.append(QString::fromStdString(it.first));
        _frameIdstrings.append(it.first);
    }

    endResetModel();
}

QModelIndex FrameListModel::toModelIndex(const idstring& frameId) const
{
    QString id = QString::fromStdString(frameId);
    int row = _frameNames.indexOf(id);

    return createIndex(row, 0);
}

idstring FrameListModel::toFrameId(int row) const
{
    if (row < 0 || row >= _frameIdstrings.size()) {
        return idstring();
    }
    return _frameIdstrings.at(row);
}

idstring FrameListModel::toFrameId(const QModelIndex& index) const
{
    return toFrameId(index.row());
}

int FrameListModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return _frameNames.size();
}

QVariant FrameListModel::data(const QModelIndex& index, int role) const
{
    if (index.row() < 0 || index.row() >= _frameNames.size()) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return _frameNames.at(index.row());
    }

    return QVariant();
}

void FrameListModel::insertFrame(const idstring& id, std::unique_ptr<SI::Frame> frame)
{
    auto& frames = _document->frameSet()->frames;

    Q_ASSERT(frames.contains(id) == false);
    Q_ASSERT(frame != nullptr);

    int index = 0;
    for (index = 0; index < _frameIdstrings.size(); index++) {
        if (_frameIdstrings.at(index).str() > id.str()) {
            break;
        }
    }

    const SI::Frame* framePtr = frame.get();

    beginInsertRows(QModelIndex(), index, index);

    frames.insertInto(id, std::move(frame));
    _frameNames.insert(index, QString::fromStdString(id));
    _frameIdstrings.insert(index, id);

    endInsertRows();

    emit _document->frameAdded(framePtr);
}

std::unique_ptr<SI::Frame> FrameListModel::removeFrame(const idstring& id)
{
    auto& frames = _document->frameSet()->frames;
    Q_ASSERT(frames.contains(id));

    emit _document->frameAboutToBeRemoved(frames.getPtr(id));

    int index = _frameIdstrings.indexOf(id);

    beginRemoveRows(QModelIndex(), index, index);

    auto frame = frames.extractFrom(id);
    _frameNames.removeAt(index);
    _frameIdstrings.removeAt(index);

    endRemoveRows();

    return frame;
}
