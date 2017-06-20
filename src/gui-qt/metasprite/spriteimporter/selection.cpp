/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include "document.h"

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Selection::Selection(QObject* parent)
    : QObject(parent)
    , _document(nullptr)
    , _selectedFrame(nullptr)
    , _selectedFrameId()
{
}

void Selection::setDocument(Document* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    unselectFrame();

    connect(_document, &Document::frameAboutToBeRemoved, this, &Selection::onFrameAboutToBeRemoved);
    connect(_document, &Document::frameRenamed, this, &Selection::onFrameRenamed);
}

void Selection::unselectFrame()
{
    _selectedFrame = nullptr;
    _selectedFrameId = idstring();

    emit selectedFrameChanged();
}

void Selection::selectFrame(const idstring& id)
{
    SI::Frame* selectedFrame = nullptr;

    if (_document) {
        selectedFrame = _document->frameSet()->frames.getPtr(id);
    }

    if (_selectedFrame != selectedFrame) {
        _selectedFrame = selectedFrame;
        _selectedFrameId = selectedFrame ? id : idstring();

        emit selectedFrameChanged();
    }
}

void Selection::onFrameAboutToBeRemoved(const SI::Frame* frame)
{
    if (_selectedFrame == frame) {
        unselectFrame();
    }
}

void Selection::onFrameRenamed(const SI::Frame* frame, const idstring& newId)
{
    if (_selectedFrame == frame) {
        _selectedFrameId = newId;
    }
}
