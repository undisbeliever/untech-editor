/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framelistmodel.h"
#include "document.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameListModel::FrameListModel(QObject* parent)
    : AbstractIdmapListModel(parent)
    , _document(nullptr)
{
}

void FrameListModel::setDocument(Document* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    buildLists(_document->frameSet()->frames);
}

void FrameListModel::insertFrame(const idstring& id, std::unique_ptr<SI::Frame> frame)
{
    auto& frames = _document->frameSet()->frames;
    const SI::Frame* framePtr = frame.get();

    insertMapItem(frames, id, std::move(frame));

    emit _document->frameAdded(framePtr);
    emit _document->frameMapChanged();
}

std::unique_ptr<SI::Frame> FrameListModel::removeFrame(const idstring& id)
{
    auto& frames = _document->frameSet()->frames;

    emit _document->frameAboutToBeRemoved(frames.getPtr(id));
    auto ret = removeMapItem(frames, id);
    emit _document->frameMapChanged();

    return ret;
}

void FrameListModel::renameFrame(const idstring& oldId, const idstring& newId)
{
    auto& frames = _document->frameSet()->frames;

    renameMapItem(frames, oldId, newId);

    emit _document->frameRenamed(frames.getPtr(newId), newId);
    emit _document->frameMapChanged();
}
