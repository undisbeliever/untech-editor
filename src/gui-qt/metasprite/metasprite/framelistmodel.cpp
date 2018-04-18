/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framelistmodel.h"
#include "accessors.h"
#include "document.h"

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

FrameListModel::FrameListModel(QObject* parent)
    : AbstractIdmapListModel(parent)
    , _document(nullptr)
{
}

void FrameListModel::setDocument(Document* document)
{
    if (_document) {
        _document->frameMap()->disconnect(this);
    }
    _document = document;

    if (_document) {
        buildLists(_document->frameSet()->frames);

        connect(_document->frameMap(), &FrameMap::itemAdded,
                this, &FrameListModel::addIdstring);
        connect(_document->frameMap(), &FrameMap::itemAboutToBeRemoved,
                this, &FrameListModel::removeIdstring);
        connect(_document->frameMap(), &FrameMap::itemRenamed,
                this, &FrameListModel::renameIdstring);
    }
    else {
        clear();
    }
}
