/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "tabbar.h"
#include "gui-qt/abstractproject.h"
#include "gui-qt/abstractresourceitem.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QVariant>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::Resources;

TabBar::TabBar(QWidget* parent)
    : QWidget(parent)
    , _tabBar(new QTabBar(this))
    , _project(nullptr)
{
    QVBoxLayout* vertical = new QVBoxLayout(this);
    vertical->setMargin(0);
    vertical->setSpacing(0);
    vertical->addWidget(_tabBar);

    _tabBar->setShape(QTabBar::TriangularNorth);
    _tabBar->setExpanding(false);
    _tabBar->setDocumentMode(true);
    _tabBar->setTabsClosable(true);
    _tabBar->setMovable(true);

    connect(_tabBar, &QTabBar::currentChanged,
            this, &TabBar::onTabBarCurrentIndexChanged);
    connect(_tabBar, &QTabBar::tabMoved,
            this, &TabBar::onTabMoved);
    connect(_tabBar, &QTabBar::tabCloseRequested,
            this, &TabBar::onTabCloseRequested);
}

void TabBar::setProject(AbstractProject* project)
{
    if (_project == project) {
        return;
    }

    if (_project != nullptr) {
        _project->disconnect(this);
    }
    _project = project;

    resetTabs();

    if (_project) {
        connect(_project, &AbstractProject::selectedResourceChanged,
                this, &TabBar::onSelectedResourceChanged);

        connect(_project, &AbstractProject::resourceItemAboutToBeRemoved,
                this, &TabBar::onResourceItemAboutToBeRemoved);
    }
}

void TabBar::resetTabs()
{
    for (AbstractResourceItem* item : _tabResources) {
        if (item) {
            item->disconnect(this);
        }
    }
    _tabResources.clear();

    for (int i = _tabBar->count(); i > 0; i--) {
        _tabBar->removeTab(i - 1);
    }

    if (_project) {
        _tabResources.append(nullptr);
        _tabBar->addTab("Project");
    }
}

AbstractResourceItem* TabBar::currentResource() const
{
    int index = _tabBar->currentIndex();
    if (index >= 0) {
        return resourceAt(index);
    }
    else {
        return nullptr;
    }
}

AbstractResourceItem* TabBar::resourceAt(int index) const
{
    Q_ASSERT(_tabResources.size() == _tabBar->count());
    Q_ASSERT(index >= 0 && index < _tabResources.size());
    Q_ASSERT(qvariant_cast<QObject*>(_tabBar->tabData(index)) == _tabResources.at(index));

    return _tabResources.at(index);
}

int TabBar::findResourceTab(AbstractResourceItem* item)
{
    if (item == nullptr) {
        return 0;
    }

    int index = _tabResources.indexOf(item);
    if (index >= 0) {
        Q_ASSERT(qvariant_cast<QObject*>(_tabBar->tabData(index)) == item);
    }

    return index;
}

int TabBar::addResourceTab(AbstractResourceItem* item)
{
    Q_ASSERT(item != nullptr);
    Q_ASSERT(_tabResources.contains(item) == false);
    Q_ASSERT(_tabResources.size() == _tabBar->count());

    int index = _tabResources.count();

    _tabResources.append(item);
    _tabBar->addTab(item->name());
    _tabBar->setTabData(index, QVariant::fromValue(item));

    updateTabText(item);

    if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(item)) {
        _tabBar->setTabToolTip(index, exItem->relativeFilePath());

        connect(exItem, &AbstractExternalResourceItem::relativeFilePathChanged,
                this, &TabBar::onItemRelativeFilePathChanged);
    }
    else {
        _tabBar->setTabToolTip(index, item->resourceList()->resourceTypeNameSingle());
    }

    connect(item, &AbstractResourceItem::nameChanged,
            this, &TabBar::onItemNameChanged);

    connect(item->undoStack(), &QUndoStack::cleanChanged,
            this, &TabBar::onItemUndoStackCleanChanged);

    return index;
}

void TabBar::updateTabText(AbstractResourceItem* item)
{
    if (item == nullptr) {
        return;
    }

    int index = findResourceTab(item);
    if (index >= 0) {
        QString s = item->name();
        if (item->undoStack()->isClean() == false) {
            s.prepend('*');
        }
        _tabBar->setTabText(index, s);
    }
}

void TabBar::removeResourceTab(int index)
{
    Q_ASSERT(index >= 0);
    Q_ASSERT(index < _tabResources.size());
    Q_ASSERT(_tabResources.size() == _tabBar->count());
    Q_ASSERT(qvariant_cast<QObject*>(_tabBar->tabData(index)) == _tabResources.at(index));

    AbstractResourceItem* item = _tabResources.at(index);
    if (item) {
        item->disconnect(this);

        _tabResources.removeAt(index);
        _tabBar->removeTab(index);
    }
}

void TabBar::onSelectedResourceChanged()
{
    Q_ASSERT(_project != nullptr);

    AbstractResourceItem* selected = _project->selectedResource();

    int index = findResourceTab(selected);
    if (index < 0) {
        index = addResourceTab(selected);
    }
    _tabBar->setCurrentIndex(index);
}

void TabBar::onTabBarCurrentIndexChanged()
{
    if (_project) {
        _project->setSelectedResource(currentResource());
    }
}

void TabBar::onTabMoved(int from, int to)
{
    _tabResources.move(from, to);
}

void TabBar::onTabCloseRequested(int index)
{
    AbstractResourceItem* item = resourceAt(index);

    if (item) {
        if (unsavedChangesDialog(item)) {
            removeResourceTab(index);
        }
    }
    else {
        emit closeProjectRequested();
    }
}

bool TabBar::unsavedChangesDialog(AbstractResourceItem* item)
{
    Q_ASSERT(item);

    bool success = true;

    if (item->undoStack()->isClean() == false) {
        success = false;

        QMessageBox dialog(QMessageBox::Warning,
                           tr("Save Changes?"),
                           tr("There are unsaved changes. Do you want to save?"),
                           QMessageBox::Save | QMessageBox::Cancel,
                           this);
        dialog.exec();

        if (dialog.result() == QMessageBox::Save) {
            try {
                if (auto* exItem = qobject_cast<AbstractExternalResourceItem*>(item)) {
                    exItem->saveResource();
                    success = true;
                }
                else {
                    auto& filename = item->project()->filename();
                    success = item->project()->saveDocument(filename);
                }
            }
            catch (const std::exception& ex) {
                QMessageBox::critical(nullptr, tr("Error Saving File"), ex.what());
                success = false;
            }
        }
    }

    return success;
}

void TabBar::onResourceItemAboutToBeRemoved(AbstractResourceItem* item)
{
    int index = findResourceTab(item);
    if (index >= 0) {
        removeResourceTab(index);
    }
}

void TabBar::onItemRelativeFilePathChanged()
{
    auto* item = qobject_cast<AbstractExternalResourceItem*>(sender());

    if (item == nullptr) {
        return;
    }

    int index = findResourceTab(item);
    if (index >= 0) {
        _tabBar->setTabToolTip(index, item->relativeFilePath());
    }
}

void TabBar::onItemNameChanged()
{
    updateTabText(qobject_cast<AbstractResourceItem*>(sender()));
}

void TabBar::onItemUndoStackCleanChanged()
{
    auto* undoStack = qobject_cast<QUndoStack*>(sender());

    if (undoStack) {
        updateTabText(qobject_cast<AbstractResourceItem*>(undoStack->parent()));
    }
}
