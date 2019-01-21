/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetfile.h"
#include "models/common/errorlist.h"
#include "models/common/string.h"
#include "models/common/validateunique.h"
#include "utsi2utms/utsi2utms.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

void FrameSetFile::setTypeFromExtension()
{
    if (filename.empty()) {
        type = FrameSetType::UNKNOWN;
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

bool FrameSetFile::convertSpriteImporter(ErrorList& errors, bool strict)
{
    if (type == FrameSetType::SPRITE_IMPORTER && siFrameSet) {
        const auto origListSize = errors.list().size();

        Utsi2Utms converter(errors);
        msFrameSet = converter.convert(*siFrameSet);

        if (strict && errors.list().size() != origListSize) {
            msFrameSet = nullptr;
            return false;
        }

        return msFrameSet == nullptr;
    }
    else {
        return true;
    }
}

void FrameSetFile::loadFile()
{
    msFrameSet = nullptr;
    siFrameSet = nullptr;

    if (filename.empty()) {
        return;
    }

    switch (type) {
    case FrameSetType::METASPRITE:
        msFrameSet = MetaSprite::loadFrameSet(filename);
        break;

    case FrameSetType::SPRITE_IMPORTER:
        siFrameSet = SpriteImporter::loadFrameSet(filename);
        break;

    case FrameSetType::UNKNOWN:
        throw std::runtime_error("Cannot load " + filename + ": Unknown frameset type");
        break;
    }
}

const idstring& FrameSetFile::name() const
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

const std::string& FrameSetFile::displayName() const
{
    if (msFrameSet) {
        return msFrameSet->name;
    }
    if (siFrameSet) {
        return siFrameSet->name;
    }
    return filename;
}

bool UnTech::MetaSprite::validateFrameSetNamesUnique(const std::vector<FrameSetFile>& frameSets,
                                                     ErrorList& err)
{
    const idstring countString("count");

    bool valid = true;

    for (auto it = frameSets.begin(); it != frameSets.end(); it++) {
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
