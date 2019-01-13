/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/abstractproject.h"
#include "gui-qt/accessor/accessor.h"
#include "models/project/project.h"
#include "models/resources/resources.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class PaletteResourceList;

namespace PRO = UnTech::Project;
namespace RES = UnTech::Resources;

class ResourceProject : public AbstractProject {
    Q_OBJECT

    static constexpr int PALETTE_LIST_INDEX = 0;

public:
    using DataT = PRO::ProjectFile;

public:
    explicit ResourceProject(QObject* parent = nullptr);
    ~ResourceProject() = default;

    PRO::ProjectFile* resourcesFile() const { return _resourcesFile.get(); }

    PaletteResourceList* paletteResourceList() const;

private:
    friend class Accessor::ProjectSettingsUndoHelper<ResourceProject>;
    PRO::ProjectFile* dataEditable() const { return _resourcesFile.get(); }

protected:
    // can throw exceptions
    virtual bool saveProjectFile(const QString& filename) final;
    virtual bool loadProjectFile(const QString& filename) final;

private:
    std::unique_ptr<PRO::ProjectFile> _resourcesFile;
};
}
}
}
