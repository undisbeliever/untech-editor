/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <sigc++/signal.h>
#include <utility>

namespace UnTech {
namespace MetaSprite {

enum class SelectedType {
    NONE = 0,
    FRAME,
    TILE_HITBOX,
    FRAME_OBJECT,
    ACTION_POINT,
    ENTITY_HITBOX
};

template <class BaseControllerT>
class SelectedController {
public:
    SelectedController(BaseControllerT& controller);

    inline SelectedType type() const { return _type; }
    std::pair<SelectedType, size_t> typeAndIndex() const;

    void selectNone();
    void selectFrame();
    void selectTileHitbox();

    // If passed SelectedType::NONE then a frame is still selected
    void selectFrameItem(const idstring& frameId, SelectedType type, size_t index);

    const std::string& typeString() const;

    bool canCreateSelected() const;
    bool canCloneSelected() const;
    bool canRemoveSelected() const;
    bool canMoveSelectedUp() const;
    bool canMoveSelectedDown() const;

    void createNewOfSelectedType();
    void cloneSelected();
    void removeSelected();
    void moveSelectedUp();
    void moveSelectedDown();

    auto& signal_selectedChanged() { return _signal_selectedChanged; }
    auto& signal_listChanged() { return _signal_listChanged; }

private:
    BaseControllerT& _controller;

    SelectedType _type;

    sigc::signal<void> _signal_selectedChanged;
    sigc::signal<void> _signal_listChanged;
};
}
}
