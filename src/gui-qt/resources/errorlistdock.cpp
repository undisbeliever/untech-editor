/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "errorlistdock.h"
#include "gui-qt/abstractproject.h"
#include "gui-qt/abstractresourceitem.h"
#include "gui-qt/resources/errorlistdock.ui.h"

using namespace UnTech::GuiQt::Resources;

ErrorListDock::ErrorListDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::ErrorListDock)
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
    Q_ASSERT(_currentItem);

    const auto& errorList = _currentItem->errorList();
    QStringList el;

    for (const auto& e : errorList.errors()) {
        el.append(QString::fromStdString((e)));
    }

    const auto& invalidTiles = errorList.invalidImageTiles();
    if (!invalidTiles.empty()) {
        el.append(tr("%1 invalid tiles:").arg(invalidTiles.size()));

        for (const auto& t : invalidTiles) {
            QString s = QString::fromLatin1("\t");
            if (t.showFrameId()) {
                s += tr("Frame %1, ").arg(t.frameId);
            }
            s += tr("Tile%1 @ %2px, %3px: %4").arg(t.size).arg(t.x).arg(t.y).arg(QString::fromUtf8(t.reasonString()));
            el.append(s);
        }
    }

    _ui->errorList->clear();
    _ui->errorList->addItems(el);
    _ui->errorList->setEnabled(!el.isEmpty());

    if (!el.isEmpty() && this->isVisible() == false) {
        this->show();
    }
}
