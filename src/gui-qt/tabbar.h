/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QList>
#include <QTabBar>
#include <QWidget>

namespace UnTech {
namespace GuiQt {
class Project;
class AbstractResourceItem;

class TabBar : public QWidget {
    Q_OBJECT

public:
    explicit TabBar(QWidget* parent = nullptr);
    ~TabBar() = default;

    void setProject(Project* project);

private:
    void resetTabs();

    AbstractResourceItem* currentResource() const;
    AbstractResourceItem* resourceAt(int index) const;

    int findResourceTab(AbstractResourceItem* item);
    int addResourceTab(AbstractResourceItem* item);
    void removeResourceTab(int index);

    void updateTabText(AbstractResourceItem* item);
    bool unsavedChangesDialog(AbstractResourceItem* item);

private slots:
    void onSelectedResourceChanged();
    void onTabBarCurrentIndexChanged();

    void onTabMoved(int from, int to);
    void onTabCloseRequested(int index);

    void onResourceItemAboutToBeRemoved(AbstractResourceItem* item);

    void onItemRelativeFilePathChanged();
    void onItemNameChanged();
    void onItemUndoStackCleanChanged();

signals:
    void closeProjectRequested();

private:
    QTabBar* const _tabBar;

    Project* _project;
    QList<AbstractResourceItem*> _tabResources;
};
}
}
