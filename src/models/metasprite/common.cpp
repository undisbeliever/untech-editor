/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "common.h"
#include "metasprite-error.h"
#include "models/common/iterators.h"
#include <cassert>

namespace UnTech::MetaSprite {

std::u8string_view NameReference::flipStringSuffix() const
{
    using namespace std::string_view_literals;

    if (hFlip && vFlip) {
        return u8" (hvFlip)"sv;
    }
    else if (hFlip) {
        return u8" (hFlip)"sv;
    }
    else if (vFlip) {
        return u8" (vFlip)"sv;
    }
    else {
        return u8""sv;
    }
}

std::shared_ptr<const ActionPointMapping>
generateActionPointMapping(const NamedList<ActionPointFunction>& apFunctions, ErrorList& err)
{
    bool valid = true;
    auto addError = [&](const auto... msg) {
        err.addErrorString(msg...);
        valid = false;
    };
    auto addApfError = [&](const unsigned index, const auto... msg) {
        err.addError(std::make_unique<ActionPointFunctionError>(ApfErrorType::ACTION_POINT_FUNCTIONS, index, stringBuilder(msg...)));
        valid = false;
    };

    if (apFunctions.empty()) {
        addError(u8"Expected at least one action point function");
        return nullptr;
    }

    if (apFunctions.size() > MAX_ACTION_POINT_FUNCTIONS) {
        addError(u8"Too many action point functions (max ", MAX_ACTION_POINT_FUNCTIONS, u8")");
        return nullptr;
    }

    auto ret = std::make_shared<ActionPointMapping>();

    ret->reserve(apFunctions.size());

    for (auto [i, apf] : const_enumerate(apFunctions)) {
        const unsigned romValue = (i + 1) * 2;
        assert(romValue <= 255 - 2);

        if (not apf.name.isValid()) {
            addApfError(i, u8"Missing action point function name");
        }

        auto success = ret->emplace(apf.name, romValue);

        if (success.second == false) {
            addApfError(i, u8"Action point function name already exists: ", apf.name);
        }
    }

    if (not valid) {
        ret = nullptr;
    }

    return ret;
}

}
