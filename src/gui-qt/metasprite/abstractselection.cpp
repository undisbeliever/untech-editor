/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractselection.h"
#include "abstractdocument.h"
#include "models/metasprite/common.h"

#include <algorithm>

using namespace UnTech::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite;

AbstractSelection::AbstractSelection(QObject* parent)
    : QObject(parent)
    , _document(nullptr)
    , _selectedFramePtr(nullptr)
    , _selectedFrameId()
    , _selectedItems()
{
}

void AbstractSelection::setDocument(AbstractDocument* document)
{
    Q_ASSERT(document != nullptr);

    if (_document) {
        _document->disconnect(this);
    }
    _document = document;

    unselectFrame();
    unselectAnimation();

    connect(_document, &AbstractDocument::frameAboutToBeRemoved,
            this, &AbstractSelection::onFrameAboutToBeRemoved);
    connect(_document, &AbstractDocument::frameRenamed,
            this, &AbstractSelection::onFrameRenamed);
    connect(_document, &AbstractDocument::frameObjectAboutToBeRemoved,
            this, &AbstractSelection::onFrameObjectAboutToBeRemoved);
    connect(_document, &AbstractDocument::frameObjectAboutToBeRemoved,
            this, &AbstractSelection::onActionPointAboutToBeRemoved);
    connect(_document, &AbstractDocument::frameObjectAboutToBeRemoved,
            this, &AbstractSelection::onEntityHitboxAboutToBeRemoved);
    connect(_document, &AbstractDocument::frameContentsMoved,
            this, &AbstractSelection::onFrameContentsMoved);
    connect(_document, &AbstractDocument::animationAboutToBeRemoved,
            this, &AbstractSelection::onAnimationAboutToBeRemoved);
    connect(_document, &AbstractDocument::animationRenamed,
            this, &AbstractSelection::onAnimationRenamed);
    connect(_document, &AbstractDocument::animationFrameAboutToBeRemoved,
            this, &AbstractSelection::onAnimationFrameAboutToBeRemoved);
    connect(_document, &AbstractDocument::animationFrameMoved,
            this, &AbstractSelection::onAnimationFrameMoved);
}

void AbstractSelection::selectFrame(const idstring& id)
{
    const void* selectedFrame = setSelectedFrame(id);

    if (_selectedFramePtr != selectedFrame) {
        _selectedFramePtr = selectedFrame;
        _selectedFrameId = selectedFrame ? id : idstring();
        _selectedItems.clear();

        emit selectedItemsChanged();
        emit selectedFrameChanged();
    }
}

void AbstractSelection::unselectFrame()
{
    _selectedFramePtr = nullptr;
    _selectedFrameId = idstring();
    _selectedItems.clear();

    emit selectedItemsChanged();
    emit selectedFrameChanged();
}

void AbstractSelection::selectAnimation(const idstring& id)
{
    MSA::Animation* ani = _document->animations()->getPtr(id);

    if (_selectedAnimation != ani) {
        _selectedAnimation = ani;
        _selectedAnimationId = ani ? id : idstring();

        _selectedAnimationFrame = -1;

        emit selectedAnimationChanged();
        emit selectedAnimationFrameChanged();
    }
}

void AbstractSelection::unselectAnimation()
{
    _selectedAnimation = nullptr;
    _selectedAnimationId = idstring();
    _selectedAnimationFrame = -1;

    emit selectedAnimationChanged();
    emit selectedAnimationFrameChanged();
}

void AbstractSelection::selectAnimationFrame(int index)
{
    if (_selectedAnimation == nullptr
        || index < 0
        || (unsigned)index >= _selectedAnimation->frames.size()) {
        index = -1;
    }

    if (_selectedAnimationFrame != index) {
        _selectedAnimationFrame = index;

        emit selectedAnimationFrameChanged();
    }
}

void AbstractSelection::onFrameAboutToBeRemoved(const void* frame)
{
    if (_selectedFramePtr == frame) {
        unselectFrame();
    }
}

void AbstractSelection::onFrameRenamed(const void* frame, const idstring& newId)
{
    if (_selectedFramePtr == frame) {
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

void AbstractSelection::onAnimationAboutToBeRemoved(const void* animation)
{
    if (_selectedAnimation == animation) {
        unselectAnimation();
    }
}

void AbstractSelection::onAnimationRenamed(const void* animation, const idstring& newId)
{
    if (_selectedAnimation == animation) {
        _selectedAnimationId = newId;
    }
}

void AbstractSelection::onAnimationFrameAboutToBeRemoved(const void* animation, unsigned index)
{
    if (_selectedAnimation == animation
        && _selectedAnimationFrame == int(index)) {

        unselectAnimation();
    }
}

void AbstractSelection::onAnimationFrameMoved(const void* animation, unsigned oldPos, unsigned newPos)
{
    if (_selectedAnimation == animation) {
        if (_selectedAnimationFrame == int(oldPos)) {
            selectAnimationFrame(newPos);
        }
        else if (_selectedAnimationFrame == int(newPos)) {
            selectAnimationFrame(oldPos);
        }
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
