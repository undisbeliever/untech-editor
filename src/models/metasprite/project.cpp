/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"
#include "models/common/errorlist.h"
#include "models/common/string.h"
#include "models/common/validateunique.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

bool Project::validateNamesUnique(ErrorList& errors) const
{
    bool valid = true;

    if (frameSets.size() > MAX_FRAMESETS) {
        errors.addError("Too many frameSets");
    }

    valid &= validateFrameSetNamesUnique(frameSets, errors);
    valid &= validateFilesAndNamesUnique(exportOrders, "export order", errors);

    return valid;
}
