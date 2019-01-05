/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlistdock.h"
#include "gui-qt/abstractproject.h"
#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/errorlistdock.ui.h"

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
}

ErrorListDock::~ErrorListDock() = default;

void ErrorListDock::setProject(AbstractProject* project)
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
        connect(_project, &AbstractProject::selectedResourceChanged,
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

void ErrorListDock::updateErrorList()
{
    using ErrorType = ErrorList::ErrorType;

    Q_ASSERT(_currentItem);

    _ui->errorList->clear();

    const auto& errorList = _currentItem->errorList().list();
    for (auto& e : errorList) {
        auto addItem = [&](const std::string& text) {
            auto* item = new QListWidgetItem(
                e.type == ErrorType::ERROR ? _errorIcon : _warningIcon,
                QString::fromStdString(text),
                _ui->errorList);

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
