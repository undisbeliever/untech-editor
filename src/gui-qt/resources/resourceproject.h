/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractproject.h"
#include "models/resources/resources.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class PaletteResourceList;

namespace RES = UnTech::Resources;

class ResourceProject : public AbstractProject {
    Q_OBJECT

    static constexpr int PALETTE_LIST_INDEX = 0;

public:
    explicit ResourceProject(QObject* parent = nullptr);
    ~ResourceProject() = default;

    RES::ResourcesFile* resourcesFile() const { return _resourcesFile.get(); }

    PaletteResourceList* paletteResourceList() const;

protected:
    // can throw exceptions
    virtual bool saveProjectFile(const QString& filename) final;
    virtual bool loadProjectFile(const QString& filename) final;

private:
    std::unique_ptr<RES::ResourcesFile> _resourcesFile;
};
}
}
}