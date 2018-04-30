/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/accessor.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace Animation {

class AnimationsMap : public QObject {
    Q_OBJECT

public:
    using DataT = MSA::Animation;
    using MapT = MSA::Animation::map_t;
    using KeyT = idstring;

private:
    AbstractMsDocument* const _document;

    idstring _selectedId;
    MSA::Animation* _selectedItem;

public:
    AnimationsMap(AbstractMsDocument* document);
    ~AnimationsMap() = default;

    AbstractMsDocument* resourceItem() const { return _document; }

    static QString typeName() { return tr("Animation"); }

    QStringList animationNames() const;

    const idstring& selectedId() const { return _selectedId; }
    const MSA::Animation* selectedItem() const { return _selectedItem; }
    const MSA::Animation* selectedAnimation() const { return _selectedItem; }

    MSA::Animation* selectedAnimation() { return _selectedItem; }

    const MapT* map()
    {
        return _document->animations();
    }

public slots:
    void setSelectedId(const idstring& id);
    void unselectItem();

protected:
    friend class Accessor::IdmapUndoHelper<AnimationsMap>;
    MapT* getMap()
    {
        return _document->animations();
    }

    friend class AnimationFramesList;
    MSA::Animation* selectedItemEditable() const { return _selectedItem; }

signals:
    void dataChanged(const MSA::Animation*);
    void mapChanged();

    void mapAboutToChange();
    void itemAdded(const idstring& id);
    void itemAboutToBeRemoved(const idstring& id);
    void itemRenamed(const idstring& oldId, const idstring& newId);

    void selectedItemChanged();
};

class AnimationFramesList : public QObject {
    Q_OBJECT

public:
    using DataT = MSA::AnimationFrame;
    using ListT = std::vector<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<MSA::Animation*>;

    constexpr static index_type max_size = UnTech::MetaSprite::MAX_ANIMATION_FRAMES;

private:
    AbstractMsDocument* const _document;

    MSA::Animation* _animation;

public:
    AnimationFramesList(AbstractMsDocument* document);
    ~AnimationFramesList() = default;

    AbstractMsDocument* resourceItem() const { return _document; }

    QString typeName() const { return tr("Animation Frame"); }

protected:
    friend class Accessor::ListUndoHelper<AnimationFramesList>;
    ListT* getList(MSA::Animation* ani)
    {
        if (ani == nullptr) {
            return nullptr;
        }
        return &ani->frames;
    }

    ArgsT selectedListTuple() const
    {
        return std::tie(_animation);
    }

signals:
    void dataChanged(const MSA::Animation*, index_type index);
    void listChanged(const MSA::Animation*);

    void listAboutToChange(const MSA::Animation*);
    void itemAdded(const MSA::Animation*, index_type index);
    void itemAboutToBeRemoved(const MSA::Animation*, index_type index);
    void itemMoved(const MSA::Animation*, index_type from, index_type to);

    void selectedListChanged();
};

using AnimationUndoHelper = Accessor::IdmapAndSelectionUndoHelper<AnimationsMap>;
using AnimationFramesUndoHelper = Accessor::ListUndoHelper<AnimationFramesList>;
}
}
}
}
