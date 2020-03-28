/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlistdock.h"
#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/errorlistdock.ui.h"
#include "gui-qt/project.h"

using namespace UnTech::GuiQt;

ErrorListDock::ErrorListDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::ErrorListDock)
    , _errorIcon(":icons/resource-error.svg")
    , _warningIcon(":icons/resource-warning.svg")
    , _project(nullptr)
    , _currentItem(nullptr)
{
    _ui->setupUi(this);

    setEnabled(false);

    connect(_ui->errorList, &QListWidget::itemDoubleClicked,
            this, &ErrorListDock::onItemDoubleClicked);
}

ErrorListDock::~ErrorListDock() = default;

void ErrorListDock::setProject(Project* project)
{
    if (_project == project) {
        return;
    }

    _ui->errorList->clear();

    if (_project != nullptr) {
        _project->disconnect(this);
    }
    _project = project;

    if (_currentItem) {
        _currentItem->disconnect(this);
    }
    _currentItem = nullptr;

    if (_project) {
        onSelectedResourceChanged();
        connect(_project, &Project::selectedResourceChanged,
                this, &ErrorListDock::onSelectedResourceChanged);
    }

    setEnabled(_project != nullptr);
}

void ErrorListDock::onSelectedResourceChanged()
{
    Q_ASSERT(_project);

    _ui->errorList->clear();

    if (_currentItem) {
        _currentItem->disconnect(this);
    }
    _currentItem = _project->selectedResource();

    if (_currentItem) {
        updateErrorList();
        connect(_currentItem, &AbstractResourceItem::errorListChanged,
                this, &ErrorListDock::updateErrorList);
    }
}

void ErrorListDock::onItemDoubleClicked(const QListWidgetItem* item)
{
    if (_currentItem == nullptr || item == nullptr) {
        return;
    }
    const auto& errorList = _currentItem->errorList().list();

    bool ok;
    unsigned errorId = item->data(errorIdRole).toUInt(&ok);

    if (ok && errorId < errorList.size()) {
        emit errorDoubleClicked(errorList.at(errorId));
    }
}

void ErrorListDock::updateErrorList()
{
    using ErrorType = ErrorListItem::ErrorType;

    Q_ASSERT(_currentItem);

    _ui->errorList->clear();

    const auto& errorList = _currentItem->errorList().list();
    for (unsigned errorId = 0; errorId < errorList.size(); errorId++) {
        const auto& e = errorList.at(errorId);

        auto addItem = [&](const std::string& text) {
            auto* item = new QListWidgetItem(
                e.type == ErrorType::ERROR ? _errorIcon : _warningIcon,
                QString::fromStdString(text),
                _ui->errorList);
            item->setData(errorIdRole, errorId);
            _ui->errorList->addItem(item);
        };

        if (!e.message.empty()) {
            addItem(e.message);
        }
        if (e.specialized) {
            addItem(e.specialized->message());
        }
    }

    _ui->errorList->setEnabled(!errorList.empty());

    if (!errorList.empty() && this->isVisible() == false) {
        this->show();
    }
}
