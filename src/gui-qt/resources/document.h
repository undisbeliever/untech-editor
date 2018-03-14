/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractdocument.h"
#include "models/resources/resources.h"
#include <array>
#include <memory>

namespace UnTech {
namespace GuiQt {
namespace Resources {
class AbstractResourceList;
class AbstractResourceItem;
class AbstractExternalResourceItem;
class ResourceValidationWorker;

namespace RES = UnTech::Resources;

class Document : public AbstractDocument {
    Q_OBJECT

public:
    static constexpr unsigned N_RESOURCE_TYPES = 2;

public:
    explicit Document(QObject* parent = nullptr);
    ~Document() = default;

    RES::ResourcesFile* resourcesFile() const { return _resourcesFile.get(); }

    const auto& resourceLists() const { return _resourceLists; }
    ResourceValidationWorker* validationWorker() const { return _validationWorker; }

    void setSelectedResource(AbstractResourceItem* item);
    AbstractResourceItem* selectedResource() const { return _selectedResource; }

    QList<AbstractExternalResourceItem*> unsavedExternalResources() const;

    // All unsaved filenames are relative to the directory this document is saved to
    QStringList unsavedFilenames() const;

    virtual const QString& fileFilter() const final;
    virtual const QString& defaultFileExtension() const final;

protected:
    // can throw exceptions
    virtual bool saveDocumentFile(const QString& filename) final;
    virtual bool loadDocumentFile(const QString& filename) final;

private:
    void initModels();

private slots:
    void onSelectedResourceDestroyed(QObject* obj);

signals:
    void selectedResourceChanged();
    void resourceItemCreated(AbstractResourceItem*);
    void resourceItemAboutToBeRemoved(AbstractResourceItem*);

private:
    std::unique_ptr<RES::ResourcesFile> _resourcesFile;

    // order matches ResourceTypeIndex
    std::array<AbstractResourceList*, N_RESOURCE_TYPES> _resourceLists;
    ResourceValidationWorker* const _validationWorker;

    AbstractResourceItem* _selectedResource;
};
}
}
}
