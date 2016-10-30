#include "rgba.h"
#include "humantypename.h"

using namespace UnTech;

static_assert(sizeof(rgba) == 4, "rgba is the wrong size");

template <>
const std::string HumanTypeName<rgba>::value = "rgba";
