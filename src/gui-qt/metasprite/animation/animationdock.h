/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/metasprite/animation/animation.h"
#include <QDockWidget>
#include <QItemSelection>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {
class AbstractDocument;

namespace Animation {
namespace Ui {
class AnimationDock;
}

namespace MSA = UnTech::MetaSprite::Animation;

class AnimationDock : public QDockWidget {
    Q_OBJECT

public:
    explicit AnimationDock(QWidget* parent = nullptr);
    ~AnimationDock();

    void setDocument(AbstractDocument* document);

    void clearGui();

private slots:
    void onSelectedAnimationChanged();
    void onSelectedAnimationFrameChanged();

    void updateGui();

    void onAnimationListSelectionChanged();
    void onAnimationFrameSelectionChanged();

private:
    std::unique_ptr<Ui::AnimationDock> _ui;
    AbstractDocument* _document;
};
}
}
}
}
