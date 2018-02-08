/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractdocument.h"
#include "models/resources/resources.h"
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {

namespace RES = UnTech::Resources;

class Document : public AbstractDocument {
    Q_OBJECT

public:
    explicit Document(QObject* parent = nullptr);
    ~Document() = default;

    RES::ResourcesFile* resourcesFile() const { return _resourcesFile.get(); }

    virtual const QString& fileFilter() const final;
    virtual const QString& defaultFileExtension() const final;

protected:
    virtual bool saveDocumentFile(const QString& filename) final;
    virtual bool loadDocumentFile(const QString& filename) final;

private:
    void initModels();

private:
    std::unique_ptr<RES::ResourcesFile> _resourcesFile;
};
}
}
}
