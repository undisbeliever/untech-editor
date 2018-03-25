/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractproject.h"
#include "models/metasprite/project.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

class MetaSpriteProject : public AbstractProject {
    Q_OBJECT

public:
    explicit MetaSpriteProject(QObject* parent = nullptr);
    ~MetaSpriteProject() = default;

    UnTech::MetaSprite::Project* metaSpriteProject() const { return _metaSpriteProject.get(); }

protected:
    // can throw exceptions
    virtual bool saveProjectFile(const QString& filename) final;
    virtual bool loadProjectFile(const QString& filename) final;

private:
    std::unique_ptr<UnTech::MetaSprite::Project> _metaSpriteProject;
};
}
}
}
