/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
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
    static const std::filesystem::path utmsExtension(".utms");
    static const std::filesystem::path utsiExtension(".utsi");

    if (filename.empty()) {
        type = FrameSetType::UNKNOWN;
        return;
    }

    if (filename.extension() == utmsExtension) {
        type = FrameSetType::METASPRITE;
    }
    else if (filename.extension() == utsiExtension) {
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

        msFrameSet = utsi2utms(*siFrameSet, errors);

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
        throw std::runtime_error(stringBuilder("Cannot load ", filename.string(), ": Unknown frameset type"));
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

const idstring& FrameSetFile::exportOrder() const
{
    static const idstring empty;

    if (msFrameSet) {
        return msFrameSet->exportOrder;
    }
    if (siFrameSet) {
        return siFrameSet->exportOrder;
    }
    return empty;
}

bool UnTech::MetaSprite::validateFrameSetNamesUnique(const std::vector<FrameSetFile>& frameSets,
                                                     ErrorList& err)
{
    const idstring countString("count");

    bool valid = true;

    for (auto it = frameSets.begin(); it != frameSets.end(); it++) {
        const std::filesystem::path& filename = it->filename;
        bool dupFn = std::any_of(it + 1, frameSets.end(),
                                 [&](const auto& i) { return i.filename == filename; });
        if (dupFn) {
            err.addErrorString("Duplicate frameset file detected: ", filename.string());
            valid = false;
            continue;
        }

        const idstring& name = it->name();

        if (name.isValid() == false) {
            auto d = std::distance(frameSets.begin(), it);
            err.addErrorString("Missing name in frameset ", d);
            valid = false;
            continue;
        }

        if (name == countString) {
            err.addErrorString("Invalid frameset name: count");
            valid = false;
            continue;
        }

        bool dup = std::any_of(it + 1, frameSets.end(),
                               [&](const auto& i) { return i.name() == name; });
        if (dup) {
            err.addErrorString("Duplicate frameset name detected: ", name);
            valid = false;
        }
    }

    return valid;
}
