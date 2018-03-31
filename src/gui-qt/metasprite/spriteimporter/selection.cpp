/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include "document.h"
#include <algorithm>

using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Selection::Selection(Document* document)
    : AbstractSelection(document)
    , _document(document)
    , _selectedFrame(nullptr)
{
}

void Selection::selectFrame(const SI::Frame* frame)
{
    if (_document->frameSet() == nullptr) {
        unselectFrame();
        return;
    }

    if (_selectedFrame != frame) {
        idstring frameId = _document->frameSet()->frames.getId(frame);
        selectFrame(frameId);
    }
}

const void* Selection::setSelectedFrame(const idstring& id)
{
    if (_document->frameSet()) {
        _selectedFrame = _document->frameSet()->frames.getPtr(id);
    }
    else {
        _selectedFrame = nullptr;
    }

    _selectedFrame = _document->frameSet()->frames.getPtr(id);
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
