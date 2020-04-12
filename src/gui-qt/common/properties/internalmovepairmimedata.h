/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QMimeData>
#include <QPair>

namespace UnTech {
namespace GuiQt {

struct InternalMovePairMimeData {
    const static QString MIME_TYPE;

    static QMimeData* toMimeData(const QPair<int, int>& index, const void* model);
    static inline QMimeData* toMimeData(const int pIndex, const int cIndex, const void* model)
    {
        return toMimeData({ pIndex, cIndex }, model);
    }

    static QPair<int, int> fromMimeData(const QMimeData* mimeData, const void* model);
};

}
}
