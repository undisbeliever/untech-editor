#include "basecontroller.h"

#include <cassert>
#include <iostream>

using namespace UnTech::Controller;

BaseController::BaseController(std::unique_ptr<ControllerInterface> interface)
    : _undoStack()
    , _interface(std::move(interface))
{
    assert(_interface != nullptr);
}

void BaseController::showError(const char* error, const std::exception& ex)
{
    std::cerr << "ERROR: " << error
              << "\n\t" << ex.what()
              << std::endl;

    if (_interface) {
        _interface->showError(error, ex);
    }
}

bool BaseController::dontMergeNextAction()
{
    _undoStack.dontMergeNextAction();
    return false;
}
