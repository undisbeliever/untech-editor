/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractprojectloader.h"
#include "gui-qt/resources/resourceproject.h"

namespace UnTech {
namespace GuiQt {
namespace Resources {

class ResourceProjectLoader : public AbstractProjectLoader {
    Q_OBJECT

public:
    ResourceProjectLoader(QObject* parent = nullptr)
        : AbstractProjectLoader(
              tr("Resource Project"),
              QStringLiteral("UnTech Resources Project (*.utres)"),
              QStringLiteral("utres"),
              parent)
    {
    }
    ~ResourceProjectLoader() = default;

    virtual std::unique_ptr<AbstractProject> newProject() final
    {
        return std::make_unique<ResourceProject>();
    }
};
}
}
}
