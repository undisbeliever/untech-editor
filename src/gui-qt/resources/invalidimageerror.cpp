/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "invalidimageerror.h"
#include "gui-qt/common/graphics/qpenhelper.h"
#include <QApplication>

using namespace UnTech::GuiQt::Resources;

namespace UnTech {
namespace GuiQt {
namespace Resources {

static const QString& reasonString(const RES::InvalidImageError::InvalidTileReason& reason)
{
    using ITR = RES::InvalidImageError::InvalidTileReason;

    static const QString NULL_STRING;
    static const QString NO_PALETTE_FOUND = QApplication::tr("No palette found");
    static const QString NOT_SAME_PALETTE = QApplication::tr("Must use the same palette in each frame");
    static const QString TOO_MANY_COLORS = QApplication::tr("Too many colors");

    switch (reason) {
    case ITR::NO_PALETTE_FOUND:
        return NO_PALETTE_FOUND;

    case ITR::NOT_SAME_PALETTE:
        return NOT_SAME_PALETTE;

    case ITR::TOO_MANY_COLORS:
        return TOO_MANY_COLORS;
    };

    return NULL_STRING;
}

void createGraphicsItemsForImageError(const RES::InvalidImageError& imageErr, QGraphicsItem* parent)
{
    // ::TODO move into Style::
    constexpr QColor ERROR_COLOR = QColor(255, 0, 50, 128);

    Q_ASSERT(QGuiApplication::topLevelWindows().size() > 0);
    QWindow* widget = QGuiApplication::topLevelWindows().first();

    QBrush errorBrush(ERROR_COLOR);
    QPen errorPen = createCosmeticPen(ERROR_COLOR, widget);

    for (const auto& tile : imageErr.invalidTiles()) {
        QRectF r(tile.x, tile.y, tile.size, tile.size);

        QGraphicsRectItem* tileItem = new QGraphicsRectItem(r, parent);
        tileItem->setBrush(errorBrush);
        tileItem->setPen(errorPen);
        tileItem->setToolTip(QStringLiteral("(%1, %2) %3").arg(tile.x).arg(tile.y).arg(reasonString(tile.reason)));
    }
}

}
}
}
