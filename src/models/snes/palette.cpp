#include "palette.h"
#include "models/common/humantypename.h"

using namespace UnTech;
using namespace UnTech::Snes;

template <>
const std::string HumanTypeName<Palette<2>>::value = "Palette";

template <>
const std::string HumanTypeName<Palette<4>>::value = "Palette";

template <>
const std::string HumanTypeName<Palette<8>>::value = "Palette";
