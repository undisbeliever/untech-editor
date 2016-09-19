#include "project.h"
#include "utsi2utms/utsi2utms.h"

using namespace UnTech;
using namespace UnTech::MetaSprite;

template <>
const std::string Project::FrameSetFile::list_t::HUMAN_TYPE_NAME = "FrameSet File";

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
