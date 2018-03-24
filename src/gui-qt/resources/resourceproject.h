/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractproject.h"
#include "gui-qt/common/abstractdocument.h"
#include "models/resources/resources.h"
#include <array>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {

namespace RES = UnTech::Resources;

class ResourceProject : public AbstractProject {
    Q_OBJECT

public:
    explicit ResourceProject(QObject* parent = nullptr);
    ~ResourceProject() = default;

    RES::ResourcesFile* resourcesFile() const { return _resourcesFile.get(); }

    virtual const QString& fileFilter() const final;
    virtual const QString& defaultFileExtension() const final;

protected:
    // can throw exceptions
    virtual bool saveDocumentFile(const QString& filename) final;
    virtual bool loadDocumentFile(const QString& filename) final;

private:
    std::unique_ptr<RES::ResourcesFile> _resourcesFile;
};
}
}
}
