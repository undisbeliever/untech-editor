/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "basecontroller.h"

#include <cassert>
#include <iostream>

using namespace UnTech::Controller;

BaseController::BaseController(ControllerInterface& interface)
    : _interface(interface)
    , _undoStack()
    , _filename()
{
}

bool BaseController::saveDocument()
{
    return saveDocument(_filename);
}

bool BaseController::saveDocument(const std::string& filename)
{
    if (!filename.empty()) {
        try {
            doSave(filename);
            _filename = filename;
            _undoStack.markClean();
            return true;
        }
        catch (const std::exception& ex) {
            showError("Unable to save", ex);
            return false;
        }
    }
    else {
        return false;
    }
}

bool BaseController::loadDocument(const std::string& filename)
{
    try {
        doLoad(filename);
        _filename = filename;
        _undoStack.clear();
        return true;
    }
    catch (const std::exception& ex) {
        showError("Unable to load", ex);
        return false;
    }
}

void BaseController::newDocument()
{
    doNew();
    _filename = "";
    _undoStack.clear();
}

void BaseController::showError(const std::exception& ex)
{
    std::cerr << "ERROR: " << ex.what()
              << std::endl;

    _interface.showError("Error", ex);
}

void BaseController::showError(const char* error, const std::exception& ex)
{
    std::cerr << "ERROR: " << error
              << "\n\t" << ex.what()
              << std::endl;

    _interface.showError(error, ex);
}
