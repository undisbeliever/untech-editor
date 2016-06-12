#pragma once

#include "gui/undo/undostack.h"
#include <gtkmm/window.h>
#include <sigc++/sigc++.h>
#include <stdexcept>

namespace UnTech {
namespace Controller {

// ::TODO separate window from Controller::

class BaseController {
public:
    BaseController(Gtk::Window& window)
        : _undoStack()
        , _window(window)
    {
    }
    BaseController(const BaseController&) = delete;
    ~BaseController() = default;

    Gtk::Window& window() { return _window; }

    Undo::UndoStack& undoStack() { return _undoStack; }
    const Undo::UndoStack& undoStack() const { return _undoStack; }

    void showError(const char* error, const std::exception& ex);

    // Always returns false
    bool dontMergeNextAction();

private:
    Undo::UndoStack _undoStack;
    Gtk::Window& _window;
};
}
}
