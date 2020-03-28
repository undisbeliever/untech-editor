/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "common.h"
#include <cassert>

using namespace UnTech;
using namespace UnTech::MetaSprite;

std::string NameReference::str() const
{
    if (hFlip && vFlip) {
        return stringBuilder(name, " (hvFlip)");
    }
    else if (hFlip) {
        return stringBuilder(name, " (hFlip)");
    }
    else if (vFlip) {
        return stringBuilder(name, " (vFlip)");
    }
    else {
        return name;
    }
}

ActionPointMapping MetaSprite::generateActionPointMapping(const NamedList<ActionPointFunction>& apFunctions, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    auto addApfError = [&](const ActionPointFunction& apf, const auto... msg) {
        err.addError(std::make_unique<ListItemError>(&apf, msg...));
        valid = false;
    };

    ActionPointMapping ret;

    if (apFunctions.empty()) {
        addError("Expected at least one action point function");
        return ret;
    }

    if (apFunctions.size() > MAX_ACTION_POINT_FUNCTIONS) {
        addError("Too many action point functions (max ", MAX_ACTION_POINT_FUNCTIONS, ")");
        return ret;
    }

    ret.reserve(apFunctions.size());

    for (unsigned i = 0; i < apFunctions.size(); i++) {
        const unsigned romValue = (i + 1) * 2;
        assert(romValue <= 255 - 2);

        const ActionPointFunction& apf = apFunctions.at(i);

        if (not apf.name.isValid()) {
            addApfError(apf, "Missing action point function name");
        }

        auto success = ret.emplace(apf.name, romValue);

        if (success.second == false) {
            addApfError(apf, "Action point function name already exists: ", apf.name);
        }
    }

    if (not valid) {
        ret.clear();
    }

    return ret;
}
