/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QDockWidget>
#include <QIcon>
#include <memory>

class QListWidgetItem;

namespace UnTech {
struct ErrorListItem;

namespace GuiQt {
namespace Ui {
class ErrorListDock;
}
class Project;
class AbstractResourceItem;

class ErrorListDock : public QDockWidget {
    Q_OBJECT

    constexpr static int errorIdRole = Qt::UserRole + 1;

public:
    ErrorListDock(QWidget* parent = nullptr);
    ~ErrorListDock();

    void setProject(Project* project);

signals:
    void errorDoubleClicked(const ErrorListItem&);

private slots:
    void onSelectedResourceChanged();

    void onItemDoubleClicked(const QListWidgetItem* item);

    void updateErrorList();

private:
    std::unique_ptr<Ui::ErrorListDock> const _ui;

    const QIcon _errorIcon;
    const QIcon _warningIcon;

    Project* _project;
    AbstractResourceItem* _currentItem;
};
}
}
