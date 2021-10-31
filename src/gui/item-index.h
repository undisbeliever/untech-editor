/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech {
enum class ResourceType : unsigned;
}

namespace UnTech::Gui {

struct ItemIndex {
    ResourceType type;
    unsigned index;

    ItemIndex(ResourceType t, unsigned i)
        : type(t)
        , index(i)
    {
    }

    bool operator==(const ItemIndex&) const = default;
};

}
