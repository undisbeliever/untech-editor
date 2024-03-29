/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetfile.h"
#include "models/common/exceptions.h"
#include "models/common/string.h"
#include "models/common/validateunique.h"

namespace UnTech::MetaSprite {

void FrameSetFile::setTypeFromExtension()
{
    static const std::filesystem::path utmsExtension(u8".utms");
    static const std::filesystem::path utsiExtension(u8".utsi");

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
        throw runtime_error(u8"Cannot load ", filename.u8string(), u8": Unknown frameset type");
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

}
