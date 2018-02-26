/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QMenu>
#include <QSettings>
#include <array>

namespace UnTech {
namespace GuiQt {

class OpenRecentMenu : public QMenu {
    Q_OBJECT

public:
    static constexpr int N_RECENT_FILENAMES = 6;
    static const QString SETTING_KEY;

public:
    explicit OpenRecentMenu(QWidget* parent = 0);
    ~OpenRecentMenu() = default;

public slots:
    void addFilename(QString filename);
    void updateMenu();

private slots:
    void onMenuTriggered(QAction* action);

signals:
    void recentFileSelected(QString filename);

private:
    std::array<QAction*, N_RECENT_FILENAMES> _actions;
};
}
}
