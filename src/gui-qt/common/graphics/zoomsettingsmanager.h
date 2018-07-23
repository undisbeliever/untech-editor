/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "zoomsettings.h"
#include <QMap>
#include <QObject>
#include <QSettings>

namespace UnTech {
namespace GuiQt {

class ZoomSettingsManager : public QObject {
    Q_OBJECT

public:
    explicit ZoomSettingsManager(QObject* parent);
    ~ZoomSettingsManager() = default;

    // These functions will create a new ZoomSetting if it doesn't already exist
    ZoomSettings* get(const QString& name);
    void set(const QString& name, qreal zoom, ZoomSettings::AspectRatio aspect);

    // All ZoomSettings MUST be created before using these functions.
    // settings will be saved to the zoom group
    void readSettings(QSettings& settings);
    void saveSettings(QSettings& settings);

signals:
    void currentChanged();

private:
    QMap<QString, ZoomSettings*> _map;
    ZoomSettings* _current;
};
}
}
