/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "internalmovepairmimedata.h"

#include <QDataStream>

namespace UnTech {
namespace GuiQt {

const QString InternalMovePairMimeData::MIME_TYPE = QStringLiteral("application/x-untech-internal-move-pair");

QMimeData* InternalMovePairMimeData::toMimeData(const QPair<int, int>& index, const void* model)
{
    static_assert(sizeof(quintptr) == sizeof(model), "Bad quintptr size");

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    const quintptr modelPtr = reinterpret_cast<quintptr>(model);

    stream << index << modelPtr;

    QMimeData* mimeData = new QMimeData();
    mimeData->setData(MIME_TYPE, data);
    return mimeData;
}

QPair<int, int> InternalMovePairMimeData::fromMimeData(const QMimeData* mimeData, const void* model)
{
    QPair<int, int> index = { INT_MIN, INT_MIN };

    QByteArray data = mimeData->data(MIME_TYPE);
    if (!data.isEmpty()) {
        QDataStream stream(&data, QIODevice::ReadOnly);

        quintptr mimeModel = 0;

        stream >> index >> mimeModel;

        if (mimeModel != quintptr(model)
            || stream.status() != QDataStream::Ok
            || stream.atEnd() == false) {

            index = { INT_MIN, INT_MIN };
        }
    }

    return index;
}

}
}
