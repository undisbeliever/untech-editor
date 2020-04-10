/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QImage>

namespace UnTech {
namespace GuiQt {

inline QRect imageBoundingRect(const QImage img)
{
    int left = INT_MAX;
    int top = INT_MAX;
    int right = INT_MIN;
    int bottom = INT_MIN;
    for (int y = 0; y < img.height(); y++) {
        const QRgb* imgBits = reinterpret_cast<const QRgb*>(img.constScanLine(y));
        const QRgb* lastBits = imgBits + img.width();

        const auto firstIt = std::find_if(imgBits, lastBits,
                                          [](QRgb p) { return p != 0; });
        if (firstIt != lastBits) {
            top = std::min<int>(top, y);
            bottom = y;

            left = std::min<int>(left, std::distance(imgBits, firstIt));

            const auto lastIt = std::find_if(std::reverse_iterator(lastBits), std::reverse_iterator(firstIt),
                                             [](QRgb p) { return p != 0; });

            right = std::max<int>(right, std::distance(lastIt, std::reverse_iterator(imgBits)) - 1);
        }
    }

    if (bottom == INT_MIN) {
        return QRect();
    }
    return QRect(left, top, right - left + 1, bottom - top + 1);
}

}
}
