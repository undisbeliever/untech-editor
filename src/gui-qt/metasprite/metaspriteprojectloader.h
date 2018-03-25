/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractprojectloader.h"
#include "gui-qt/metasprite/metaspriteproject.h"

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

class MetaSpriteProjectLoader : public AbstractProjectLoader {
    Q_OBJECT

public:
    MetaSpriteProjectLoader(QObject* parent = nullptr)
        : AbstractProjectLoader(
              tr("MetaSprite Project"),
              QStringLiteral("UnTech MetaSprite Project (*.utmspro)"),
              QStringLiteral("utmspro"),
              parent)
    {
    }
    ~MetaSpriteProjectLoader() = default;

    virtual std::unique_ptr<AbstractProject> newProject() final
    {
        return std::make_unique<MetaSpriteProject>();
    }
};
}
}
}
