/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

namespace UnTech::Gui {
enum class EditorType : unsigned;

struct ItemIndex {
    EditorType type;
    unsigned index;

    ItemIndex(EditorType t, unsigned i)
        : type(t)
        , index(i)
    {
    }

    bool operator==(const ItemIndex& o) const
    {
        return this->type == o.type
               && this->index == o.index;
    }
};

}
