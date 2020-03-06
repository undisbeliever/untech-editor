/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/resources/invalid-image-error.h"
#include <QGraphicsRectItem>

namespace UnTech {
namespace GuiQt {
namespace Resources {

namespace RES = UnTech::Resources;

void createGraphicsItemsForImageError(const RES::InvalidImageError& imageErr, QGraphicsItem* parent);

}
}
}
