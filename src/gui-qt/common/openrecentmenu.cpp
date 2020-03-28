/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "openrecentmenu.h"

#include <QFileInfo>

using namespace UnTech::GuiQt;

const QString OpenRecentMenu::SETTING_KEY = QString::fromUtf8("recent_files");

OpenRecentMenu::OpenRecentMenu(QWidget* parent)
    : QMenu(tr("Open &Recent"), parent)
{
    this->setToolTipsVisible(true);

    for (auto& a : _actions) {
        a = addAction(QString());
    }
    updateMenu();

    connect(this, &QMenu::triggered,
            this, &OpenRecentMenu::onMenuTriggered);
}

void OpenRecentMenu::addFilename(QString filename)
{
    filename = QFileInfo(filename).absoluteFilePath();

    QSettings settings;
    QStringList files = settings.value(SETTING_KEY).toStringList();

    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > N_RECENT_FILENAMES) {
        files.removeLast();
    }

    settings.setValue(SETTING_KEY, files);

    updateMenu();
}

void OpenRecentMenu::updateMenu()
{
    QSettings settings;
    const QStringList files = settings.value(SETTING_KEY).toStringList();

    for (int i = 0; i < N_RECENT_FILENAMES; i++) {
        QAction* action = _actions.at(i);

        action->setVisible(i < files.size());
        if (i < files.size()) {
            const QString& file = files.at(i);
            action->setText(QFileInfo(file).fileName());
            action->setToolTip(file);
            action->setData(file);
        }
    }
}

void OpenRecentMenu::onMenuTriggered(QAction* action)
{
    QString filename = action->data().toString();
    if (!filename.isEmpty()) {
        emit recentFileSelected(filename);
    }
}
