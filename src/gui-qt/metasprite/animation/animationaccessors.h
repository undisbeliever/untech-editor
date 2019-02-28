/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/accessor/abstractaccessors.h"
#include "gui-qt/accessor/accessor.h"
#include "gui-qt/metasprite/abstractmsdocument.h"
#include <QObject>
#include <tuple>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
namespace Animation {

class AnimationsList : public Accessor::NamedListAccessor<MSA::Animation, AbstractMsDocument> {
    Q_OBJECT

    friend class Accessor::NamedListUndoHelper<AnimationsList>;

public:
    AnimationsList(AbstractMsDocument* document);
    ~AnimationsList() = default;

    virtual QString typeName() const final;

    bool editSelected_setDurationFormat(MSA::DurationFormat durationFormat);
    bool editSelected_setOneShot(bool oneShot);

    // Also sets oneShot to false
    bool editSelected_setNextAnimation(const idstring& nextAnimation);
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
