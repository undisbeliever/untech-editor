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

class AnimationsList : public QObject {
    Q_OBJECT

public:
    using DataT = MSA::Animation;
    using ListT = NamedList<MSA::Animation>;
    using index_type = ListT::size_type;

    constexpr static index_type max_size = UnTech::MetaSprite::MAX_EXPORT_NAMES;

private:
    AbstractMsDocument* const _document;

    size_t _selectedIndex;

public:
    AnimationsList(AbstractMsDocument* document);
    ~AnimationsList() = default;

    AbstractMsDocument* resourceItem() const { return _document; }

    static QString typeName() { return tr("Animation"); }

    QStringList animationNames() const;

    index_type selectedIndex() const { return _selectedIndex; }
    void setSelectedId(const idstring& id);
    void setSelectedIndex(const index_type& index);
    void unselectItem() { setSelectedIndex(INT_MAX); }

    bool isSelectedIndexValid() const;

    const MSA::Animation* selectedAnimation() const;

    bool editSelected_setDurationFormat(MSA::DurationFormat durationFormat);
    bool editSelected_setOneShot(bool oneShot);
    bool editSelected_setNextAnimation(const idstring& nextAnimation);

    const ListT* list() const { return _document->animations(); }

protected:
    friend class Accessor::NamedListUndoHelper<AnimationsList>;
    ListT* getList() { return _document->animations(); }

    friend class AnimationFramesList;
    MSA::Animation* selectedItemEditable();

signals:
    void nameChanged(index_type index);
    void dataChanged(index_type index);
    void listChanged();

    void listAboutToChange();
    void itemAdded(index_type index);
    void itemAboutToBeRemoved(index_type index);
    void itemMoved(index_type from, index_type to);

    void selectedIndexChanged();
};

class AnimationFramesList : public QObject {
    Q_OBJECT

public:
    using DataT = MSA::AnimationFrame;
    using ListT = std::vector<DataT>;
    using index_type = ListT::size_type;
    using ArgsT = std::tuple<size_t>;

    constexpr static index_type max_size = UnTech::MetaSprite::MAX_ANIMATION_FRAMES;

private:
    AbstractMsDocument* const _document;
    size_t _animationIndex;

public:
    AnimationFramesList(AbstractMsDocument* document);
    ~AnimationFramesList() = default;

    AbstractMsDocument* resourceItem() const { return _document; }

    QString typeName() const { return tr("Animation Frame"); }

    bool editSelectedList_setData(index_type index, const DataT& value);

    bool editSelectedList_addItem(index_type index);
    bool editSelectedList_cloneItem(index_type index);
    bool editSelectedList_removeItem(index_type index);
    bool editSelectedList_moveItem(index_type from, index_type to);

protected:
    friend class Accessor::ListUndoHelper<AnimationFramesList>;
    ListT* getList(size_t aniIndex)
    {
        auto* animations = _document->animations();
        if (animations == nullptr) {
            return nullptr;
        }
        if (aniIndex >= animations->size()) {
            return nullptr;
        }
        return &animations->at(aniIndex).frames;
    }

    ArgsT selectedListTuple() const
    {
        return std::tie(_animationIndex);
    }

signals:
    void dataChanged(size_t animationIndex, index_type index);
    void listChanged(size_t animationIndex);

    void listAboutToChange(size_t animationIndex);
    void itemAdded(size_t animationIndex, index_type index);
    void itemAboutToBeRemoved(size_t animationIndex, index_type index);
    void itemMoved(size_t animationIndex, index_type from, index_type to);

    void selectedListChanged();
};
}
}
}
}
