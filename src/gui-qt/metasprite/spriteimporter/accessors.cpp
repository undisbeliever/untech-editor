/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/multipleselectedindexeshelper.h"
#include "gui-qt/accessor/selectedindexhelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameList::FrameList(Document* document)
    : QObject(document)
    , _document(document)
    , _selectedIndex(INT_MAX)
    , _tileHitboxSelected(false)
{
    SelectedIndexHelper::buildAndConnectSlots_NamedList(this);
}

void FrameList::setTileHitboxSelected(bool s)
{
    if (_tileHitboxSelected != s) {
        _tileHitboxSelected = s;
        emit tileHitboxSelectedChanged();
    }
}

void FrameList::setSelectedId(const UnTech::idstring& id)
{
    if (auto* list = this->list()) {
        setSelectedIndex(list->indexOf(id));
    }
    else {
        unselectItem();
    }
}

void FrameList::setSelectedIndex(FrameList::index_type index)
{
    if (_selectedIndex != index) {
        _selectedIndex = index;
        emit selectedIndexChanged();
    }
}

bool FrameList::isFrameSelected() const
{
    auto* frames = list();
    if (frames == nullptr) {
        return false;
    }
    return _selectedIndex < frames->size();
}

const UnTech::MetaSprite::SpriteImporter::Frame* FrameList::selectedFrame() const
{
    auto* frames = list();
    if (frames == nullptr) {
        return nullptr;
    }
    if (_selectedIndex >= frames->size()) {
        return nullptr;
    }
    return &frames->at(_selectedIndex);
}

SI::Frame* FrameList::selectedItemEditable()
{
    auto* frames = getList();
    if (_selectedIndex >= frames->size()) {
        return nullptr;
    }
    return &frames->at(_selectedIndex);
}

AbstractFrameContentAccessor::AbstractFrameContentAccessor(Document* document)
    : QObject(document)
    , _document(document)
    , _selectedIndexes()
{
    MultipleSelectedIndexesHelper::buildAndConnectSlots(this);

    connect(_document->frameList(), &FrameList::selectedIndexChanged,
            this, [this]() {
                clearSelection();
                emit selectedListChanged();
            });
}

void AbstractFrameContentAccessor::setSelectedIndexes(const vectorset<index_type>& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = selected;
        emit selectedIndexesChanged();
    }
}

void AbstractFrameContentAccessor::setSelectedIndexes(vectorset<index_type>&& selected)
{
    if (_selectedIndexes != selected) {
        _selectedIndexes = std::move(selected);
        emit selectedIndexesChanged();
    }
}

void AbstractFrameContentAccessor::clearSelection()
{
    if (!_selectedIndexes.empty()) {
        _selectedIndexes.clear();
        emit selectedIndexesChanged();
    }
}

FrameObjectList::FrameObjectList(Document* document)
    : AbstractFrameContentAccessor(document)
{
}

ActionPointList::ActionPointList(Document* document)
    : AbstractFrameContentAccessor(document)

{
}

EntityHitboxList::EntityHitboxList(Document* document)
    : AbstractFrameContentAccessor(document)
{
}
