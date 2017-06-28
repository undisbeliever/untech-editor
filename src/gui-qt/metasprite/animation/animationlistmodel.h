/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractidmaplistmodel.h"
#include "models/metasprite/animation/animation.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractDocument;

namespace Animation {

class AnimationListModel : public AbstractIdmapListModel {
    Q_OBJECT

public:
    explicit AnimationListModel(QObject* parent = nullptr);
    ~AnimationListModel() = default;

    void setDocument(AbstractDocument* document);

private:
    AbstractDocument* _document;
};
}
}
}
}
