/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcestreedock.h"
#include "abstractproject.h"
#include "abstractresourceitem.h"
#include "abstractresourcelist.h"
#include "resourcestreemodel.h"
#include "gui-qt/common/idstringdialog.h"
#include "gui-qt/resources/resourcestreedock.ui.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>

using namespace UnTech::GuiQt::Resources;

ResourcesTreeDock::ResourcesTreeDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::ResourcesTreeDock)
    , _model(new ResourcesTreeModel)
    , _addResourceMenu(new QMenu(this))
    , _project(nullptr)
{
    _ui->setupUi(this);

    _ui->resourcesTree->setModel(_model);
    _ui->addResourceAction->setMenu(_addResourceMenu);

    // HACK: Required to enable the menu in the ToolBar
    for (auto* w : _ui->addResourceAction->associatedWidgets()) {
        if (auto* b = qobject_cast<QToolButton*>(w)) {
            b->setPopupMode(QToolButton::InstantPopup);
        }
    }

    setupAddResourceMenu();

    setProject(nullptr);

    connect(_ui->resourcesTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ResourcesTreeDock::onResourcesTreeSelectionChanged);

    connect(_addResourceMenu, &QMenu::triggered,
            this, &ResourcesTreeDock::onAddResourceMenuTriggered);
    connect(_ui->removeResourceAction, &QAction::triggered,
            this, &ResourcesTreeDock::onRemoveResourceTriggered);
}

void ResourcesTreeDock::setupAddResourceMenu()
{
    _addResourceMenu->setTitle(tr("Add Resource"));
    _addResourceMenu->setIcon(QIcon(":/icons/add.svg"));

    _addResourceMenu->clear();
    _addResourceMenu->setEnabled(_project != nullptr);

    if (_project) {
        for (const AbstractResourceList* rl : _project->resourceLists()) {
            QAction* m = _addResourceMenu->addAction(
                tr("Add %1").arg(rl->resourceTypeNameSingle()));

            m->setData((unsigned)rl->resourceTypeIndex());
        }
    }
}

ResourcesTreeDock::~ResourcesTreeDock() = default;

void ResourcesTreeDock::setProject(AbstractProject* project)
{
    if (_project != nullptr) {
        _project->disconnect(this);
    }
    _project = project;

    _model->setProject(project);
    _addResourceMenu->setEnabled(_project);
    _ui->addResourceAction->setEnabled(_project);
    _ui->removeResourceAction->setEnabled(false);

    setupAddResourceMenu();

    if (_project) {
        onSelectedResourceChanged();
        connect(_project, &AbstractProject::selectedResourceChanged,
                this, &ResourcesTreeDock::onSelectedResourceChanged);
    }

    setEnabled(_project != nullptr);

    _ui->resourcesTree->expandAll();
}

void ResourcesTreeDock::onAddResourceMenuTriggered(QAction* action)
{
    Q_ASSERT(_project);

    const int listId = action->data().toInt();
    Q_ASSERT(listId < _project->resourceLists().size());

    AbstractResourceList* resourceList = _project->resourceLists().at(listId);
    const auto& settings = resourceList->addResourceDialogSettings();

    QString input;
    if (!settings.filter.isEmpty() && !settings.title.isEmpty()) {
        QDir dir(resourceList->project()->filename());
        dir.cdUp();

        QFileDialog dialog(this, settings.title);
        if (settings.canCreateFile) {
            dialog.setFileMode(QFileDialog::AnyFile);
        }
        else {
            dialog.setFileMode(QFileDialog::ExistingFile);
        }
        dialog.setNameFilter(settings.filter);
        dialog.setDefaultSuffix(settings.extension);
        dialog.setDirectory(dir.absolutePath());
        dialog.exec();

        if (dialog.result() == QDialog::Accepted && !dialog.selectedFiles().empty()) {
            Q_ASSERT(!_project->filename().isEmpty());

            input = dialog.selectedFiles().first();

            const auto& items = resourceList->items();
            auto it = std::find_if(items.begin(), items.end(),
                                   [&](auto* item) { return item->filename() == input; });
            if (it != items.end()) {
                QMessageBox::critical(this,
                                      tr("Error"),
                                      tr("Cannot add Resource: Resource file already in list."));
                _project->setSelectedResource(*it);
                return;
            }

            QString relativeFn = QFileInfo(_project->filename()).absoluteDir().relativeFilePath(input);
            if (relativeFn.startsWith("..") || QFileInfo(relativeFn).isRelative() == false) {
                QMessageBox verifyDialog(QMessageBox::Warning,
                                         tr("Something is not quite right..."),
                                         tr("The file \"%1\" is outside the resource file's directory."
                                            "\n\nAre you sure you want to continue?")
                                             .arg(relativeFn),
                                         QMessageBox::Yes | QMessageBox::No,
                                         this);
                verifyDialog.setDefaultButton(QMessageBox::No);
                verifyDialog.exec();

                if (verifyDialog.result() == QMessageBox::No) {
                    return;
                }
            }
        }
    }
    else {
        idstring name = IdstringDialog::getIdstring(
            this, settings.title,
            tr("Name of the new %1:").arg(resourceList->resourceTypeNameSingle()));
        input = QString::fromStdString(name);

        if (resourceList->findResource(input) != nullptr) {
            QMessageBox::critical(this,
                                  tr("Error"),
                                  tr("%1 name already exists.").arg(resourceList->resourceTypeNameSingle()));
            return;
        }
    }

    if (!input.isEmpty()) {
        resourceList->addResource(input);
    }
}

void ResourcesTreeDock::onRemoveResourceTriggered()
{
    Q_ASSERT(_project);
    AbstractResourceItem* selected = _project->selectedResource();
    Q_ASSERT(selected);
    AbstractResourceList* resourceList = selected->resourceList();
    Q_ASSERT(resourceList);

    QMessageBox dialog(QMessageBox::Warning,
                       tr("Remove Resource?"),
                       tr("Are you sure you want to remove %1, it cannot be undone?")
                           .arg(resourceList->resourceTypeNameSingle()),
                       QMessageBox::Yes | QMessageBox::No,
                       this);
    dialog.setDefaultButton(QMessageBox::No);
    dialog.exec();
    if (dialog.result() == QMessageBox::Yes) {
        resourceList->removeResource(selected->index());
    }
}

void ResourcesTreeDock::onSelectedResourceChanged()
{
    AbstractResourceItem* selected = _project->selectedResource();

    QModelIndex index = _model->toModelIndex(selected);
    _ui->resourcesTree->setCurrentIndex(index);

    _ui->removeResourceAction->setEnabled(selected != nullptr);
}

void ResourcesTreeDock::onResourcesTreeSelectionChanged()
{
    if (_project == nullptr) {
        return;
    }

    QModelIndex index = _ui->resourcesTree->currentIndex();
    AbstractResourceItem* item = _model->toResourceItem(index);
    _project->setSelectedResource(item);
}
