/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlist.h"
#include "stringstream.h"
#include <algorithm>

namespace UnTech {

ErrorList::ErrorList()
    : _list()
    , _errorCount(0)
{
}

void ErrorList::printIndented(StringStream& out) const
{
    for (const auto& item : _list) {
        if (!item->isWarning) {
            out.write("    ERROR: ");
        }
        else {
            out.write("    Warning: ");
        }

        item->printIndented(out);

        out.write("\n");
    }
}

void AbstractError::printIndented(StringStream& out) const
{
    out.write(message);
}

}
