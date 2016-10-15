#include "animation.h"
#include "models/common/humantypename.h"

using namespace UnTech;
using namespace UnTech::MetaSprite::Animation;

template <>
const std::string HumanTypeName<Animation>::value = "Animation";
template <>
const std::string HumanTypeName<Instruction>::value = "Animation Instruction";
