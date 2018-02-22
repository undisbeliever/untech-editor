/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractresourceitem.h"

using namespace UnTech::GuiQt::Resources;

void AbstractResourceItem::markDirty()
{
    if (_state != ResourceState::NOT_LOADED && _state != ResourceState::FILE_ERROR) {
        setState(ResourceState::DIRTY);
    }
}

void AbstractResourceItem::loadResource()
{
    RES::ErrorList err;

    try {
        bool s = loadResourceData(err);
        setState(s ? ResourceState::DIRTY : ResourceState::FILE_ERROR);
    }
    catch (const std::exception& ex) {
        err.addError(std::string("EXCEPTION: ") + ex.what());
        setState(ResourceState::FILE_ERROR);
    }

    _errorList = err;
    emit errorListChanged();
}

void AbstractResourceItem::validateItem()
{
    if (_state == ResourceState::FILE_ERROR || _state == ResourceState::NOT_LOADED) {
        return;
    }

    RES::ErrorList err;

    try {
        bool s = compileResource(err);
        setState(s ? ResourceState::VALID : ResourceState::ERROR);
    }
    catch (const std::exception& ex) {
        err.addError(std::string("EXCEPTION: ") + ex.what());
        setState(ResourceState::ERROR);
    }

    _errorList = err;
    emit errorListChanged();
}

void AbstractResourceItem::setState(ResourceState state)
{
    if (_state != state) {
        _state = state;
        emit stateChanged();
    }
}
