/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include "document.h"
#include <algorithm>

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

Selection::Selection(QObject* parent)
    : AbstractSelection(parent)
    , _document(nullptr)
    , _selectedFrame(nullptr)
{
}

void Selection::setDocument(Document* document)
{
    AbstractSelection::setDocument(document);
    _document = document;
}

const void* Selection::setSelectedFrame(const idstring& id)
{
    if (_document) {
        _selectedFrame = _document->frameSet()->frames.getPtr(id);
    }
    else {
        _selectedFrame = nullptr;
    }
    return _selectedFrame;
}

unsigned Selection::nObjectsInSelectedFrame() const
{
    if (_selectedFrame) {
        return _selectedFrame->objects.size();
    }
    return 0;
}

unsigned Selection::nActionPointsInSelectedFrame() const
{
    if (_selectedFrame) {
        return _selectedFrame->actionPoints.size();
    }
    return 0;
}

unsigned Selection::nEntityHitboxesInSelectedFrame() const
{
    if (_selectedFrame) {
        return _selectedFrame->entityHitboxes.size();
    }
    return 0;
}
