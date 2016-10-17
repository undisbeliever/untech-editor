#pragma once

#include "undo/undostack.h"
#include <stdexcept>

namespace UnTech {
namespace Controller {

/**
 * Pure virtual class for interfacing between the
 * controller and the View.
 */
class ControllerInterface {
public:
    virtual ~ControllerInterface() = default;

    virtual void showError(const char* error, const std::exception& ex) = 0;
};

class BaseController {
public:
    BaseController(ControllerInterface&);
    virtual ~BaseController() = default;

    BaseController(const BaseController&) = delete;

    Undo::UndoStack& undoStack() { return _undoStack; }
    const Undo::UndoStack& undoStack() const { return _undoStack; }

    const std::string& filename() const { return _filename; }

    virtual bool hasDocument() const = 0;

    // returns false if cannot save.
    bool saveDocument();
    bool saveDocument(const std::string& filename);
    bool loadDocument(const std::string& filename);
    void newDocument();

    // Prints the error to stderr, then calls `ControllerInterface::showError`
    void showError(const std::exception& ex);
    void showError(const char* error, const std::exception& ex);

protected:
    // throw exception on error
    virtual void doSave(const std::string& filename) = 0;

    // throw exception on error
    virtual void doLoad(const std::string& filename) = 0;

    // throw exception on error
    virtual void doNew() = 0;

private:
    ControllerInterface& _interface;
    Undo::UndoStack _undoStack;
    std::string _filename;
};
}
}
