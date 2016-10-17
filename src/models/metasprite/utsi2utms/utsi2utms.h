#pragma once

#include "../errorlist.h"
#include "../metasprite.h"
#include "../spriteimporter.h"
#include <list>
#include <memory>
#include <string>

namespace UnTech {
namespace MetaSprite {

class Utsi2Utms {
public:
    Utsi2Utms(ErrorList& errorList);

    std::unique_ptr<MetaSprite::FrameSet> convert(SpriteImporter::FrameSet& siFrameSet);

    // NOTE: updates the frame Locations
    bool validateFrameSet(SpriteImporter::FrameSet&);

private:
    std::unique_ptr<MetaSprite::FrameSet> process(const SpriteImporter::FrameSet& siFrameSet);

private:
    ErrorList& errorList;
};
}
}
