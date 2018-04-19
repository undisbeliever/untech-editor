/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "animationaccessors.h"
#include "gui-qt/undo/selectedidmapitemhelper.h"
#include "gui-qt/undo/selectedindexhelper.h"

using namespace UnTech::GuiQt::Undo;
using namespace UnTech::GuiQt::MetaSprite;
using namespace UnTech::GuiQt::MetaSprite::Animation;

AnimationsMap::AnimationsMap(AbstractMsDocument* document)
    : QObject(document)
    , _document(document)
    , _selectedId()
    , _selectedItem(nullptr)
{
    SelectedIdmapItemHelper::buildAndConnectSlots(this);
}

QStringList AnimationsMap::animationNames() const
{
    QStringList al;

    if (const auto* aniMap = _document->animations()) {
        al.reserve(aniMap->size());
        for (const auto& it : *aniMap) {
            al.append(QString::fromStdString(it.first));
        }
    }

    return al;
}

void AnimationsMap::setSelectedId(const UnTech::idstring& id)
{
    if (_selectedId != id) {
        MapT* map = getMap();
        if (map == nullptr) {
            unselectItem();
            return;
        }

        _selectedItem = map->getPtr(id);
        _selectedId = _selectedItem ? id : idstring();

        emit selectedItemChanged();
    }
}

void AnimationsMap::unselectItem()
{
    if (_selectedId.isValid() || _selectedItem != nullptr) {
        _selectedId = idstring();
        _selectedItem = nullptr;

        emit selectedItemChanged();
    }
}

AnimationFramesList::AnimationFramesList(AbstractMsDocument* document)
    : QObject(document)
    , _document(document)
    , _animation(nullptr)
{
    connect(_document->animationsMap(), &AnimationsMap::selectedItemChanged,
            this, [this] {
                _animation = _document->animationsMap()->selectedItemEditable();
                emit selectedListChanged();
            });
}
