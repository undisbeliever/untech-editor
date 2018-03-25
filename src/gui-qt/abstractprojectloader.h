/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "abstractproject.h"
#include <QObject>
#include <memory>

namespace UnTech {
namespace GuiQt {
class AbstractProject;

class AbstractProjectLoader : public QObject {
    Q_OBJECT

public:
    AbstractProjectLoader(const QString& name, const QString& fileFilter,
                          const QString& fileExtension,
                          QObject* parent)
        : QObject(parent)
        , _name(name)
        , _fileFilter(fileFilter)
        , _fileExtension(fileExtension)
    {
    }
    ~AbstractProjectLoader() = default;

    const QString& name() const { return _name; }
    const QString& fileFilter() const { return _fileFilter; }
    const QString& fileExtension() const { return _fileExtension; }

    virtual std::unique_ptr<AbstractProject> newProject() = 0;

    std::unique_ptr<AbstractProject> loadProject(const QString& filename)
    {
        std::unique_ptr<AbstractProject> project = newProject();
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
