/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "gui-qt/common/abstractdocument.h"
#include <array>
#include <memory>

namespace UnTech {
namespace GuiQt {
class AbstractResourceList;
class AbstractResourceItem;
class AbstractExternalResourceItem;
class ResourceValidationWorker;

class AbstractProject : public AbstractDocument {
    Q_OBJECT

public:
    explicit AbstractProject(QObject* parent = nullptr);
    ~AbstractProject() = default;

protected:
    void initResourceLists(std::initializer_list<AbstractResourceList*> resourceLists);

public:
    const auto& resourceLists() const { return _resourceLists; }
    ResourceValidationWorker* validationWorker() const { return _validationWorker; }

    void setSelectedResource(AbstractResourceItem* item);
    AbstractResourceItem* selectedResource() const { return _selectedResource; }

    QList<AbstractExternalResourceItem*> unsavedExternalResources() const;

    // All unsaved filenames are relative to the directory this project is saved to
    QStringList unsavedFilenames() const;

    virtual const QString& fileFilter() const = 0;
    virtual const QString& defaultFileExtension() const = 0;

protected:
    // can throw exceptions
    virtual bool saveDocumentFile(const QString& filename) = 0;
    virtual bool loadDocumentFile(const QString& filename) = 0;

protected:
    void rebuildResourceLists();

private slots:
    void onSelectedResourceDestroyed(QObject* obj);

signals:
    void resourceFileSettingsChanged();

    void selectedResourceChanged();
    void resourceItemCreated(AbstractResourceItem*);
    void resourceItemAboutToBeRemoved(AbstractResourceItem*);

private:
    QList<AbstractResourceList*> _resourceLists;
    ResourceValidationWorker* const _validationWorker;

    AbstractResourceItem* _selectedResource;
};
}
}
