/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractselection.h"
#include "abstractmsdocument.h"
#include "gui-qt/metasprite/animation/animationaccessors.h"
#include "models/metasprite/common.h"

#include <algorithm>

using namespace UnTech::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite;

std::set<SelectedItem>
UnTech::GuiQt::MetaSprite::moveSelectedItems(
    const std::set<SelectedItem>& items, int offset)
{
    std::set<SelectedItem> ret;
    for (const SelectedItem& item : items) {
        if (item.type == SelectedItem::NONE) {
            ret.insert({ SelectedItem::NONE, 0 });
        }
        else if (offset > 0 || item.index > 0) {
            ret.insert({ item.type, item.index + offset });
        }
        else {
            ret.insert(item);
        }
    }
    return ret;
}

AbstractSelection::AbstractSelection(AbstractMsDocument* document)
    : QObject(document)
    , _document(document)
    , _selectedFramePtr(nullptr)
    , _selectedFrameId()
    , _selectedItems()
{
    Q_ASSERT(document);

    connect(_document, qOverload<const void*>(&AbstractMsDocument::frameAboutToBeRemoved),
            this, &AbstractSelection::onFrameAboutToBeRemoved);
    connect(_document, &AbstractMsDocument::frameRenamed,
            this, &AbstractSelection::onFrameRenamed);
    connect(_document, &AbstractMsDocument::frameObjectAboutToBeRemoved,
            this, &AbstractSelection::onFrameObjectAboutToBeRemoved);
    connect(_document, &AbstractMsDocument::frameObjectAboutToBeRemoved,
            this, &AbstractSelection::onActionPointAboutToBeRemoved);
    connect(_document, &AbstractMsDocument::frameObjectAboutToBeRemoved,
            this, &AbstractSelection::onEntityHitboxAboutToBeRemoved);
    connect(_document, &AbstractMsDocument::frameContentsMoved,
            this, &AbstractSelection::onFrameContentsMoved);
}

void AbstractSelection::unselectAll()
{
    unselectFrame();
}

void AbstractSelection::selectFrame(const idstring& id)
{
    if (_selectedFrameId != id) {
        if (!_selectedItems.empty()) {
            _selectedItems.clear();
            emit selectedItemsChanged();
        }

        const void* selectedFrame = setSelectedFrame(id);

        if (_selectedFramePtr != selectedFrame) {
            _selectedFramePtr = selectedFrame;
            _selectedFrameId = selectedFrame ? id : idstring();

            emit selectedFrameChanged();
        }
    }
}

void AbstractSelection::unselectFrame()
{
    selectFrame(idstring());
}

void AbstractSelection::onFrameAboutToBeRemoved(const void* frame)
{
    if (_selectedFramePtr == frame) {
        unselectFrame();
    }
}

void AbstractSelection::onFrameRenamed(const idstring& oldId, const idstring& newId)
{
    if (_selectedFrameId == oldId) {
        _selectedFrameId = newId;
    }
}

void AbstractSelection::onFrameObjectAboutToBeRemoved(const void* frame, unsigned index)
{
    if (_selectedFramePtr == frame) {
        _selectedItems.erase({ SelectedItem::FRAME_OBJECT, index });
    }
}

void AbstractSelection::onActionPointAboutToBeRemoved(const void* frame, unsigned index)
{
    if (_selectedFramePtr == frame) {
        _selectedItems.erase({ SelectedItem::ACTION_POINT, index });
    }
}

void AbstractSelection::onEntityHitboxAboutToBeRemoved(const void* frame, unsigned index)
{
    if (_selectedFramePtr == frame) {
        _selectedItems.erase({ SelectedItem::ENTITY_HITBOX, index });
    }
}

void AbstractSelection::onFrameContentsMoved(const void* frame,
                                             const std::set<SelectedItem>& oldPos, int offset)
{
    if (_selectedFramePtr == frame) {
        const std::set<SelectedItem> newPos = moveSelectedItems(oldPos, offset);
        std::set<SelectedItem> selection;

        for (const SelectedItem& item : _selectedItems) {
            auto it = oldPos.find(item);
            if (it != oldPos.end()) {
                selection.insert({ item.type, item.index + offset });
            }
            else {
                SelectedItem i = item;
                while (newPos.find(i) != newPos.end()) {
                    i.index -= offset;
                }
                selection.insert(i);
            }
        }

        setSelectedItems(selection);
    }
}

bool AbstractSelection::canCloneSelectedItems() const
{
    if (_selectedFramePtr == nullptr || _selectedItems.size() == 0) {
        return false;
    }

    unsigned nObjects = 0;
    unsigned nActionPoints = 0;
    unsigned nEntityHitboxes = 0;

    for (const auto& item : _selectedItems) {
        switch (item.type) {
        case SelectedItem::NONE:
        case SelectedItem::TILE_HITBOX:
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

    if (nObjects > 0 && nObjects + nObjectsInSelectedFrame() > MAX_FRAME_OBJECTS) {
        return false;
    }
    if (nActionPoints > 0 && nActionPoints + nActionPointsInSelectedFrame() > MAX_ACTION_POINTS) {
        return false;
    }
    if (nEntityHitboxes > 0 && nEntityHitboxes + nEntityHitboxesInSelectedFrame() > MAX_ENTITY_HITBOXES) {
        return false;
    }
    return true;
}

void AbstractSelection::setSelectedItems(const std::set<SelectedItem>& items)
{
    if (_selectedItems != items) {
        _selectedItems = items;
        emit selectedItemsChanged();
    }
}

bool AbstractSelection::canRaiseSelectedItems() const
{
    if (_selectedItems.size() == 0
        || _selectedFramePtr == nullptr) {
        return false;
    }

    for (const auto& item : _selectedItems) {
        if (item.index == 0 || item.type == SelectedItem::NONE) {

            return false;
        }
    }

    return true;
}

bool AbstractSelection::canLowerSelectedItems() const
{
    if (_selectedItems.size() == 0
        || _selectedFramePtr == nullptr) {
        return false;
    }

    unsigned nObjects = nObjectsInSelectedFrame();
    unsigned nActionPoints = nActionPointsInSelectedFrame();
    unsigned nEntityHitboxes = nEntityHitboxesInSelectedFrame();

    for (const auto& item : _selectedItems) {
        switch (item.type) {
        case SelectedItem::NONE:
        case SelectedItem::TILE_HITBOX:
            return false;

        case SelectedItem::FRAME_OBJECT:
            if (item.index + 1 >= nObjects) {
                return false;
            }
            break;

        case SelectedItem::ACTION_POINT:
            if (item.index + 1 >= nActionPoints) {
                return false;
            }
            break;

        case SelectedItem::ENTITY_HITBOX:
            if (item.index + 1 >= nEntityHitboxes) {
                return false;
            }
            break;
        }
    }

    return true;
}

void AbstractSelection::selectFrameObject(unsigned index)
{
    _selectedItems.clear();

    if (_selectedFramePtr && index + 1 <= nObjectsInSelectedFrame()) {
        _selectedItems.insert({ SelectedItem::FRAME_OBJECT, index });
    }

    emit selectedItemsChanged();
}

void AbstractSelection::selectActionPoint(unsigned index)
{
    _selectedItems.clear();

    if (_selectedFramePtr && index + 1 <= nActionPointsInSelectedFrame()) {
        _selectedItems.insert({ SelectedItem::ACTION_POINT, index });
    }

    emit selectedItemsChanged();
}

void AbstractSelection::selectEntityHitbox(unsigned index)
{
    _selectedItems.clear();

    if (_selectedFramePtr && index + 1 <= nEntityHitboxesInSelectedFrame()) {
        _selectedItems.insert({ SelectedItem::ENTITY_HITBOX, index });
    }

    emit selectedItemsChanged();
}

bool AbstractSelection::isFrameObjectSelected() const
{
    return std::any_of(_selectedItems.begin(), _selectedItems.end(),
                       [](auto& i) { return i.type == SelectedItem::FRAME_OBJECT; });
}

bool AbstractSelection::isActionPointSelected() const
{
    return std::any_of(_selectedItems.begin(), _selectedItems.end(),
                       [](auto& i) { return i.type == SelectedItem::ACTION_POINT; });
}

bool AbstractSelection::isEntityHitboxSelected() const
{
    return std::any_of(_selectedItems.begin(), _selectedItems.end(),
                       [](auto& i) { return i.type == SelectedItem::ENTITY_HITBOX; });
}
