/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QComboBox>
#include <QMenu>
#include <QObject>

namespace UnTech {
namespace GuiQt {

class ZoomSettings;

// This class does not contain the GUI elements in a QWidget,
// but instead exposes them so the caller can lay them out as
// they please.
class ZoomSettingsUi : public QObject {
    Q_OBJECT

    const static QRegExp ZOOM_REGEXP;

public:
    explicit ZoomSettingsUi(QObject* parent);
    ~ZoomSettingsUi() = default;

    void setZoomSettings(ZoomSettings* settings);
    ZoomSettings* zoomSettings() const { return _settings; }

    QMenu* zoomMenu() const { return _zoomMenu; }
    QMenu* aspectRatioMenu() const { return _aspectRatioMenu; }

    QComboBox* zoomComboBox() const { return _zoomComboBox; }
    QComboBox* aspectRatioComboBox() const { return _aspectRatioComboBox; }

    QAction* zoomInAction() const { return _zoomInAction; }
    QAction* zoomOutAction() const { return _zoomOutAction; }

    void setEnabled(bool enabled);

    void populateMenu(QMenu* menu);

private slots:
    void updateZoomGui();
    void updateAspectRatioGui();

    void onZoomComboBoxActivated(int index);
    void onZoomComboBoxEdited();
    void onZoomMenuTriggered(QAction* action);
    void onAspectRatioComboBoxActivated(int index);
    void onAspectRatioMenuTriggered(QAction* action);

    void onZoomInTriggered();
    void onZoomOutTriggered();

private:
    QMenu* const _zoomMenu;
    QMenu* const _aspectRatioMenu;
    QComboBox* const _zoomComboBox;
    QComboBox* const _aspectRatioComboBox;

    QAction* const _zoomInAction;
    QAction* const _zoomOutAction;

    ZoomSettings* _settings;
};
}
}
