/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlist.h"
#include <algorithm>
#include <ostream>

namespace UnTech {

ErrorList::ErrorList()
    : _list()
    , _errorCount(0)
{
}

void ErrorList::printIndented(std::ostream& out) const
{
    for (const auto& item : _list) {
        if (!item->isWarning) {
            out << "    ERROR: ";
        }
        else {
            out << "    Warning: ";
        }

        item->printIndented(out);

        out << '\n';
    }
}

void AbstractError::printIndented(std::ostream& out) const
{
    out << message;
}

}
