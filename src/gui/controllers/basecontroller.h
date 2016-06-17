#pragma once

#include "gui/controllers/undo/undostack.h"
#include <sigc++/sigc++.h>
#include <stdexcept>

namespace UnTech {
namespace Controller {

/**
 * Pure virtual class for interfacing between the
 * controller and the GUI.
 */
class ControllerInterface {
public:
    virtual void showError(const char* error, const std::exception& ex) = 0;
};

class BaseController {
public:
    BaseController(std::unique_ptr<ControllerInterface>);

    BaseController(const BaseController&) = delete;
    ~BaseController() = default;

    Undo::UndoStack& undoStack() { return _undoStack; }
    const Undo::UndoStack& undoStack() const { return _undoStack; }

    // Prints the error to stderr, then calls `ControllerInterface::showError`
    void showError(const char* error, const std::exception& ex);

    // Always returns false
    bool dontMergeNextAction();

private:
    Undo::UndoStack _undoStack;
    std::unique_ptr<ControllerInterface> _interface;
};
}
}
