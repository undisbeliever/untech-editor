/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"
#include "models/common/humantypename.h"
#include "utsi2utms/utsi2utms.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

template <>
const std::string HumanTypeName<Project::FrameSetFile>::value = "FrameSet File";

bool Project::FrameSetFile::convertSpriteImporter(ErrorList& errors, bool strict)
{
    if (type == Project::FrameSetType::SPRITE_IMPORTER && siFrameSet) {
        size_t nOrigWarnings = errors.warnings.size();

        Utsi2Utms converter(errors);
        msFrameSet = converter.convert(*siFrameSet);

        if (strict && errors.warnings.size() != nOrigWarnings) {
            msFrameSet = nullptr;
            return false;
        }

        return msFrameSet == nullptr;
    }
    else {
        return true;
    }
}
