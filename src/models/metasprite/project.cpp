/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "project.h"
#include "models/common/string.h"
#include "models/common/validateunique.h"
#include "utsi2utms/utsi2utms.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

void Project::FrameSetFile::setTypeFromExtension()
{
    if (filename.empty()) {
        type = FrameSetType::NONE;
        return;
    }

    if (String::endsWith(filename, ".utms")) {
        type = FrameSetType::METASPRITE;
    }
    else if (String::endsWith(filename, ".utsi")) {
        type = FrameSetType::SPRITE_IMPORTER;
    }
    else {
        type = FrameSetType::UNKNOWN;
    }
}

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

const idstring& Project::FrameSetFile::name() const
{
    static const idstring empty;

    if (msFrameSet) {
        return msFrameSet->name;
    }
    if (siFrameSet) {
        return siFrameSet->name;
    }
    return empty;
}

static bool validateFrameSetNamesUnique(const std::vector<Project::FrameSetFile>& frameSets,
                                        ErrorList& err)
{
    const idstring countString("count");

    bool valid = true;

    for (auto it = frameSets.begin(); it != frameSets.end(); it++) {
        if (it->type == Project::FrameSetType::NONE) {
            continue;
        }

        const std::string& filename = it->filename;
        bool dupFn = std::any_of(it + 1, frameSets.end(),
                                 [&](const auto& i) { return i.filename == filename; });
        if (dupFn) {
            err.addError("Duplicate frameset file detected: " + filename);
            valid = false;
            continue;
        }

        const idstring& name = it->name();

        if (name.isValid() == false) {
            auto d = std::distance(frameSets.begin(), it);
            err.addError("Missing name in frameset " + std::to_string(d));
            valid = false;
            continue;
        }

        if (name == countString) {
            err.addError("Invalid frameset name: count");
            valid = false;
            continue;
        }

        bool dup = std::any_of(it + 1, frameSets.end(),
                               [&](const auto& i) { return i.name() == name; });
        if (dup) {
            err.addError("Duplicate frameset name detected: " + name);
            valid = false;
        }
    }

    return valid;
}

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
