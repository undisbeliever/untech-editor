/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractresourceitem.h"

using namespace UnTech::GuiQt::Resources;

void AbstractResourceItem::markDirty()
{
    setState(ResourceState::DIRTY);
}

void AbstractResourceItem::validateItem()
{
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
    if (state != _state) {
        _state = state;
        emit stateChanged();
    }
}
