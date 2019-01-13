/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "project.h"
#include <QObject>
#include <memory>

namespace UnTech {
namespace GuiQt {
class Project;

class AbstractProjectLoader : public QObject {
    Q_OBJECT

public:
    AbstractProjectLoader(QObject* parent)
        : QObject(parent)
        , _name(tr("Resource Project"))
        , _fileFilter(QStringLiteral("UnTech Project File (*.utproject)"))
        , _fileExtension(QStringLiteral("utproject"))
    {
    }
    ~AbstractProjectLoader() = default;

    const QString& name() const { return _name; }
    const QString& fileFilter() const { return _fileFilter; }
    const QString& fileExtension() const { return _fileExtension; }

    std::unique_ptr<Project> newProject()
    {
        return std::make_unique<Project>();
    }

    std::unique_ptr<Project> loadProject(const QString& filename)
    {
        std::unique_ptr<Project> project = newProject();
        if (project->loadProject(filename)) {
            return project;
        }
        return nullptr;
    }

private:
    const QString _name;
    const QString _fileFilter;
    const QString _fileExtension;
};
}
}
