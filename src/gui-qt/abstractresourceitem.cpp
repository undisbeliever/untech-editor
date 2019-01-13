/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractresourceitem.h"
#include "abstractproject.h"
#include "resourcevalidationworker.h"

#include <QDir>
#include <QFileInfo>

using namespace UnTech::GuiQt;

void AbstractResourceItem::markUnchecked()
{
    if (_state != ResourceState::NOT_LOADED && _state != ResourceState::FILE_ERROR) {
        setState(ResourceState::UNCHECKED);
    }
}

void AbstractResourceItem::loadResource()
{
    Q_ASSERT(_undoStack->count() == 0);

    ErrorList err;

    try {
        bool s = loadResourceData(err);
        setState(s ? ResourceState::UNCHECKED : ResourceState::FILE_ERROR);
    }
    catch (const std::exception& ex) {
        err.addError(std::string("EXCEPTION: ") + ex.what());
        setState(ResourceState::FILE_ERROR);
    }

    _errorList = std::move(err);

    emit resourceLoaded();
    emit errorListChanged();
}

void AbstractResourceItem::setName(const QString& name)
{
    if (_name != name) {
        _name = name;
        emit nameChanged();
    }
}

void AbstractResourceItem::setDependencies(const QVector<Dependency>& dependencies)
{
    if (_dependencies != dependencies) {
        _dependencies = dependencies;
        markUnchecked();
    }
}

void AbstractResourceItem::removeDependencies()
{
    if (_dependencies.isEmpty() == false) {
        _dependencies.clear();
        markUnchecked();
    }
}

void AbstractResourceItem::setIndex(unsigned index)
{
    Q_ASSERT(index < (unsigned)_list->items().size());

    _index = index;
}

void AbstractResourceItem::validateItem()
{
    if (_state == ResourceState::FILE_ERROR || _state == ResourceState::NOT_LOADED) {
        return;
    }

    ErrorList err;

    // check dependencies
    bool dependenciesOk = true;
    for (auto& dep : _dependencies) {
        AbstractResourceItem* dItem = _project->findResourceItem(dep.type, dep.name);

        if (dItem == nullptr) {
            if (auto* rl = _project->findResourceList(dep.type)) {
                QString rtn = rl->resourceTypeNameSingle();
                err.addError("Dependency Error: Missing " + rtn.toStdString() + u8" · " + dep.name.toStdString());
            }
            else {
                err.addError("Dependency Error: Missing " + dep.name.toStdString());
            }
            dependenciesOk = false;
        }
        else if (dItem->state() != ResourceState::VALID) {
            QString rtn = dItem->resourceList()->resourceTypeNameSingle();
            err.addError("Dependency Error: " + rtn.toStdString() + u8" · " + dep.name.toStdString());

            dependenciesOk = false;
        }
    }

    if (dependenciesOk) {
        try {
            bool s = compileResource(err);
            setState(s ? ResourceState::VALID : ResourceState::ERROR);
        }
        catch (const std::exception& ex) {
            err.addError(std::string("EXCEPTION: ") + ex.what());
            setState(ResourceState::ERROR);
        }

        _errorList = std::move(err);
        emit errorListChanged();

        emit resourceComplied();
    }
    else {
        _errorList = std::move(err);
        emit errorListChanged();

        setState(ResourceState::DEPENDENCY_ERROR);
    }

    markDependantsUnchecked();
}

void AbstractResourceItem::markDependantsUnchecked()
{
    const Dependency toMatch = { _list->resourceTypeIndex(), _name };

    for (AbstractResourceList* rl : _project->resourceLists()) {
        for (AbstractResourceItem* item : rl->items()) {
            if (item->dependencies().contains(toMatch)) {
                item->markUnchecked();
            }
        }
    }
}

void AbstractResourceItem::setState(ResourceState state)
{
    if (_state != state) {
        _state = state;
        emit stateChanged();
    }
}

void AbstractResourceItem::setExternalFiles(const QStringList& externalFiles)
{
    if (_externalFiles != externalFiles) {
        _externalFiles = externalFiles;

        emit externalFilesChanged();
    }
}

/*
 * AbstractInternalResourceItem
 */

AbstractInternalResourceItem::AbstractInternalResourceItem(AbstractResourceList* parent, unsigned index)
    : AbstractResourceItem(parent, index)
{
    // no need to load an internal resource
    _state = ResourceState::UNCHECKED;
}

QString AbstractInternalResourceItem::filename() const
{
    return QString();
}

bool AbstractInternalResourceItem::loadResourceData(ErrorList&)
{
    return true;
}

/*
 * AbstractExternalResourceItem
 */

AbstractExternalResourceItem::AbstractExternalResourceItem(AbstractResourceList* parent, unsigned index,
                                                           const QString& filename)
    : AbstractResourceItem(parent, index)
{
    setFilename(filename);

    connect(_project, &AbstractProject::filenameChanged,
            this, &AbstractExternalResourceItem::updateRelativePath);
}

QString AbstractExternalResourceItem::filename() const
{
    return _absoluteFilePath;
}

void AbstractExternalResourceItem::setFilename(const QString& filename)
{
    QString absFilePath;
    if (!filename.isEmpty()) {
        absFilePath = QDir::toNativeSeparators(
            QFileInfo(filename).absoluteFilePath());
    }

    if (_absoluteFilePath != absFilePath) {
        _absoluteFilePath = absFilePath;

        updateRelativePath();

        emit absoluteFilePathChanged();
    }
}

void AbstractExternalResourceItem::saveResource()
{
    Q_ASSERT(filename().isEmpty() == false);

    emit aboutToSaveResource();

    saveResourceData(filename().toStdString());
    undoStack()->setClean();
}

void AbstractExternalResourceItem::updateRelativePath()
{
    Q_ASSERT(_project);

    QString relPath = _absoluteFilePath;

    if (!_absoluteFilePath.isEmpty()) {
        const auto& docFn = _project->filename();
        if (!docFn.isEmpty()) {
            relPath = QDir::toNativeSeparators(
                QFileInfo(docFn).absoluteDir().relativeFilePath(_absoluteFilePath));
        }
        else {
            relPath = _absoluteFilePath;
        }
    }

    if (_relativeFilePath != relPath) {
        _relativeFilePath = relPath;
        emit relativeFilePathChanged();
    }
}
