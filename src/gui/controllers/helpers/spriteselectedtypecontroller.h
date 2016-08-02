#pragma once

#include <atomic>
#include <sigc++/signal.h>
#include <string>

namespace UnTech {
namespace Controller {

// Shared controller code for metasprite and sprite-importer.
template <class BaseControllerT>
class SpriteSelectedTypeController {
public:
    enum class Type {
        NONE = 0,
        FRAME_OBJECT,
        ACTION_POINT,
        ENTITY_HITBOX
    };

public:
    SpriteSelectedTypeController(BaseControllerT& controller);
    ~SpriteSelectedTypeController() = default;

    inline Type type() const { return _type; }
    const void* selectedPtr() const;

    // be careful with this one
    void selectItem(Type type, const void* ptr);

    const std::string& typeString() const;

    bool canCrudSelected() const { return _type != Type::NONE; }
    bool canMoveSelectedUp() const;
    bool canMoveSelectedDown() const;

    void createNewOfSelectedType();
    void cloneSelected();
    void removeSelected();
    void moveSelectedUp();
    void moveSelectedDown();

    auto& signal_selectedChanged() { return _signal_selectedChanged; }
    auto& signal_typeChanged() { return _signal_typeChanged; }
    auto& signal_listChanged() { return _signal_listChanged; }

protected:
    // Will unselect all not of type
    // Will emit a selectedChanged signal
    void setType(Type type);

private:
    BaseControllerT& _controller;

    Type _type = Type::NONE;
    std::atomic<bool> _updatingType;

    sigc::signal<void> _signal_selectedChanged;
    sigc::signal<void> _signal_typeChanged;
    sigc::signal<void> _signal_listChanged;
};
}
}
