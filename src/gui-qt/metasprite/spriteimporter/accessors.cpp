/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "accessors.h"
#include "gui-qt/accessor/multipleselectedindexeshelper.h"
#include "gui-qt/accessor/selectedidmapitemhelper.h"
#include "gui-qt/accessor/selectedindexhelper.h"

using namespace UnTech::GuiQt::Accessor;
using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

FrameMap::FrameMap(Document* document)
    : QObject(document)
    , _document(document)
    , _selectedId()
    , _selectedItem(nullptr)
    , _tileHitboxSelected(false)
{
    SelectedIdmapItemHelper::buildAndConnectSlots(this);
}

void FrameMap::setTileHitboxSelected(bool s)
{
    if (_tileHitboxSelected != s) {
        _tileHitboxSelected = s;
        emit tileHitboxSelectedChanged();
    }
}

void FrameMap::setSelectedId(const UnTech::idstring& id)
{
    if (_selectedId != id) {
        MapT* map = getMap();
        if (map == nullptr) {
            unselectItem();
            return;
        }

        setTileHitboxSelected(false);

        _selectedItem = map->getPtr(id);
        _selectedId = _selectedItem ? id : idstring();

        emit selectedItemChanged();
    }
}

void FrameMap::unselectItem()
{
    if (_selectedId.isValid() || _selectedItem != nullptr) {
        setTileHitboxSelected(false);

        _selectedId = idstring();
        _selectedItem = nullptr;

        emit selectedItemChanged();
    }
}

AbstractFrameContentAccessor::AbstractFrameContentAccessor(Document* document)
    : QObject(document)
    , _document(document)
    , _selectedIndexes()
{
    MultipleSelectedIndexesHelper::buildAndConnectSlots(this);

    connect(_document->frameMap(), &FrameMap::selectedItemChanged,
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
