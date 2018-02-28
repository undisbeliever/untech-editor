/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "resourcestreedock.h"
#include "document.h"
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
    , _document(nullptr)
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

    setDocument(nullptr);

    connect(_ui->resourcesTree->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &ResourcesTreeDock::onResourcesTreeSelectionChanged);

    connect(_addResourceMenu, &QMenu::triggered,
            this, &ResourcesTreeDock::onAddResourceMenuTriggered);
    connect(_ui->removeResourceAction, &QAction::triggered,
            this, &ResourcesTreeDock::onRemoveResourceTriggered);
}

void ResourcesTreeDock::setupAddResourceMenu()
{
    Document doc;

    _addResourceMenu->setTitle(tr("Add Resource"));
    _addResourceMenu->setIcon(QIcon(":/icons/add.svg"));

    _addResourceMenu->clear();
    for (const AbstractResourceList* rl : doc.resourceLists()) {
        QAction* m = _addResourceMenu->addAction(
            tr("Add %1").arg(rl->resourceTypeNameSingle()));

        m->setData((unsigned)rl->resourceTypeIndex());
    }
}

ResourcesTreeDock::~ResourcesTreeDock() = default;

void ResourcesTreeDock::setDocument(Document* document)
{
    if (_document != nullptr) {
        _document->disconnect(this);
    }
    _document = document;

    _model->setDocument(document);
    _addResourceMenu->setEnabled(_document);
    _ui->addResourceAction->setEnabled(_document);
    _ui->removeResourceAction->setEnabled(false);

    if (_document) {
        onSelectedResourceChanged();
        connect(_document, &Document::selectedResourceChanged,
                this, &ResourcesTreeDock::onSelectedResourceChanged);
    }

    setEnabled(_document != nullptr);
}

void ResourcesTreeDock::onAddResourceMenuTriggered(QAction* action)
{
    Q_ASSERT(_document);

    const unsigned listId = action->data().toUInt();
    Q_ASSERT(listId < _document->resourceLists().size());

    AbstractResourceList* resourceList = _document->resourceLists().at(listId);
    const auto& settings = resourceList->addResourceDialogSettings();

    QString input;
    if (!settings.filter.isEmpty() && !settings.title.isEmpty()) {
        QDir dir(resourceList->document()->filename());
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
            input = dialog.selectedFiles().first();
        }
    }
    else {
        idstring name = IdstringDialog::getIdstring(
            this, settings.title,
            tr("Name of the new %1:").arg(resourceList->resourceTypeNameSingle()));
        input = QString::fromStdString(name);
    }

    if (!input.isEmpty()) {
        resourceList->addResource(input);
    }
}

void ResourcesTreeDock::onRemoveResourceTriggered()
{
    Q_ASSERT(_document);
    AbstractResourceItem* selected = _document->selectedResource();
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
    AbstractResourceItem* selected = _document->selectedResource();

    QModelIndex index = _model->toModelIndex(selected);
    _ui->resourcesTree->setCurrentIndex(index);

    _ui->removeResourceAction->setEnabled(selected != nullptr);
}

void ResourcesTreeDock::onResourcesTreeSelectionChanged()
{
    if (_document == nullptr) {
        return;
    }

    QModelIndex index = _ui->resourcesTree->currentIndex();
    AbstractResourceItem* item = _model->toResourceItem(index);
    _document->setSelectedResource(item);
}
