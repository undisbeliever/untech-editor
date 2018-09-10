/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/grid.h"
#include <QGraphicsObject>
#include <cstdint>

namespace UnTech {
namespace GuiQt {
namespace MetaTiles {
class MtGraphicsScene;

class MtGridGraphicsItem : public QGraphicsObject {
    Q_OBJECT

public:
    using grid_t = UnTech::grid<uint16_t>;

public:
    MtGridGraphicsItem(MtGraphicsScene* scene);
    ~MtGridGraphicsItem() = default;

    virtual QRectF boundingRect() const override;

    virtual void paint(QPainter* painter,
                       const QStyleOptionGraphicsItem* option,
                       QWidget* widget = nullptr) final;

public slots:
    void updateAll();
    void onGridResized();

private:
    MtGraphicsScene* const _scene;
    QRectF _boundingRect;
};
}
}
}
