/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "selection.h"
#include "document.h"
#include <algorithm>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Selection::Selection(QObject* parent)
    : QObject(parent)
    , _document(nullptr)
    , _selectedFrame(nullptr)
    , _selectedFrameId()
    , _selectedItems()
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
    connect(_document, &Document::frameObjectAboutToBeRemoved,
            this, &Selection::onFrameObjectAboutToBeRemoved);
    connect(_document, &Document::frameObjectAboutToBeRemoved,
            this, &Selection::onActionPointAboutToBeRemoved);
    connect(_document, &Document::frameObjectAboutToBeRemoved,
            this, &Selection::onEntityHitboxAboutToBeRemoved);
    connect(_document, &Document::frameContentsMoved,
            this, &Selection::onFrameContentsMoved);
}

void Selection::unselectFrame()
{
    _selectedFrame = nullptr;
    _selectedFrameId = idstring();
    _selectedItems.clear();

    emit selectedItemsChanged();
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
        _selectedItems.clear();

        emit selectedItemsChanged();
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

void Selection::onFrameObjectAboutToBeRemoved(const SI::Frame* frame, unsigned index)
{
    if (_selectedFrame == frame) {
        _selectedItems.erase({ SelectedItem::FRAME_OBJECT, index });
    }
}

void Selection::onActionPointAboutToBeRemoved(const SI::Frame* frame, unsigned index)
{
    if (_selectedFrame == frame) {
        _selectedItems.erase({ SelectedItem::ACTION_POINT, index });
    }
}

void Selection::onEntityHitboxAboutToBeRemoved(const SI::Frame* frame, unsigned index)
{
    if (_selectedFrame == frame) {
        _selectedItems.erase({ SelectedItem::ENTITY_HITBOX, index });
    }
}

void Selection::onFrameContentsMoved(const SI::Frame* frame,
                                     const std::set<SelectedItem>& oldPos, int offset)
{
    if (_selectedFrame == frame) {
        std::set<SelectedItem> selection;

        for (const SelectedItem& item : _selectedItems) {
            auto it = oldPos.find(item);
            if (it != oldPos.end()) {
                selection.insert({ item.type, item.index + offset });
            }
            else {
                selection.insert(item);
            }
        }

        setSelectedItems(selection);
    }
}

bool Selection::canCloneSelectedItems() const
{
    if (_selectedFrame == nullptr || _selectedItems.size() == 0) {
        return false;
    }

    unsigned nObjects = 0;
    unsigned nActionPoints = 0;
    unsigned nEntityHitboxes = 0;

    for (const auto& item : _selectedItems) {
        switch (item.type) {
        case SelectedItem::NONE:
            return false;

        case SelectedItem::FRAME_OBJECT:
            nObjects++;
            break;

        case SelectedItem::ACTION_POINT:
            nActionPoints++;
            break;

        case SelectedItem::ENTITY_HITBOX:
            nEntityHitboxes++;
            break;
        }
    }

    return _selectedFrame->objects.can_insert(nObjects)
           && _selectedFrame->actionPoints.can_insert(nActionPoints)
           && _selectedFrame->entityHitboxes.can_insert(nEntityHitboxes);
}

void Selection::setSelectedItems(const std::set<SelectedItem>& items)
{
    if (_selectedItems != items) {
        _selectedItems = items;
        emit selectedItemsChanged();
    }
}

bool Selection::canRaiseSelectedItems() const
{
    if (_selectedItems.size() == 0
        || _selectedFrame == nullptr) {
        return false;
    }

    for (const auto& item : _selectedItems) {
        if (item.index == 0 || item.type == SelectedItem::NONE) {

            return false;
        }
    }

    return true;
}

bool Selection::canLowerSelectedItems() const
{
    if (_selectedItems.size() == 0
        || _selectedFrame == nullptr) {
        return false;
    }

    for (const auto& item : _selectedItems) {
        switch (item.type) {
        case SelectedItem::NONE:
            return false;

        case SelectedItem::FRAME_OBJECT:
            if (item.index + 1 >= _selectedFrame->objects.size()) {
                return false;
            }
            break;

        case SelectedItem::ACTION_POINT:
            if (item.index + 1 >= _selectedFrame->actionPoints.size()) {
                return false;
            }
            break;

        case SelectedItem::ENTITY_HITBOX:
            if (item.index + 1 >= _selectedFrame->entityHitboxes.size()) {
                return false;
            }
            break;
        }
    }

    return true;
}

void Selection::selectFrameObject(unsigned index)
{
    _selectedItems.clear();

    if (_selectedFrame && _selectedFrame->objects.size()) {
        _selectedItems.insert({ SelectedItem::FRAME_OBJECT, index });
    }

    emit selectedItemsChanged();
}

void Selection::selectActionPoint(unsigned index)
{
    _selectedItems.clear();

    if (_selectedFrame && _selectedFrame->actionPoints.size()) {
        _selectedItems.insert({ SelectedItem::ACTION_POINT, index });
    }

    emit selectedItemsChanged();
}

void Selection::selectEntityHitbox(unsigned index)
{
    _selectedItems.clear();

    if (_selectedFrame && _selectedFrame->entityHitboxes.size()) {
        _selectedItems.insert({ SelectedItem::ENTITY_HITBOX, index });
    }

    emit selectedItemsChanged();
}
