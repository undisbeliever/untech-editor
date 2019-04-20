/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "zoomsettingsui.h"
#include "zoomsettings.h"

#include <QLineEdit>
#include <QRegExpValidator>

using namespace UnTech::GuiQt;

ZoomSettingsUi::ZoomSettingsUi(QObject* parent)
    : QObject(parent)
    , _zoomMenu(new QMenu(tr("Zoom")))
    , _aspectRatioMenu(new QMenu(tr("Aspect Ratio")))
    , _zoomComboBox(new QComboBox())
    , _aspectRatioComboBox(new QComboBox())
    , _zoomInAction(new QAction(this))
    , _zoomOutAction(new QAction(this))
    , _settings(nullptr)
{
    setEnabled(false);

    {
        QActionGroup* zoomActionGroup = new QActionGroup(_zoomMenu);
        zoomActionGroup->setExclusive(true);

        for (qreal z : ZoomSettings::ZOOM_LEVELS) {
            QAction* a = zoomActionGroup->addAction(QString::fromUtf8("%1%").arg(z * 100));
            a->setData(z);
            a->setCheckable(true);
            _zoomMenu->addAction(a);
        }

        connect(_zoomMenu, &QMenu::triggered,
                this, &ZoomSettingsUi::onZoomMenuTriggered);
    }
    {
        QActionGroup* aspectRatioGroup = new QActionGroup(_aspectRatioMenu);
        aspectRatioGroup->setExclusive(true);

        auto addAspectAction = [&](const char* title, ZoomSettings::AspectRatio ratio) {
            QAction* a = aspectRatioGroup->addAction(tr(title));
            a->setData(ratio);
            a->setCheckable(true);
            _aspectRatioMenu->addAction(a);
        };
        addAspectAction("Square", ZoomSettings::SQUARE);
        addAspectAction("NTSC", ZoomSettings::NTSC);
        addAspectAction("PAL", ZoomSettings::PAL);

        connect(_aspectRatioMenu, &QMenu::triggered,
                this, &ZoomSettingsUi::onAspectRatioMenuTriggered);
    }
    {
        for (qreal z : ZoomSettings::ZOOM_LEVELS) {
            QString s = QString::fromUtf8("%1%").arg(z * 100);
            _zoomComboBox->addItem(s, z);
        }
        _zoomComboBox->setEditable(true);
        _zoomComboBox->setInsertPolicy(QComboBox::NoInsert);
        _zoomComboBox->setValidator(new QRegExpValidator(ZoomSettings::ZOOM_REGEXP, _zoomComboBox));

        connect(_zoomComboBox, qOverload<int>(&QComboBox::activated),
                this, &ZoomSettingsUi::onZoomComboBoxActivated);
        connect(_zoomComboBox->lineEdit(), &QLineEdit::editingFinished,
                this, &ZoomSettingsUi::onZoomComboBoxEdited);
    }
    {
        _aspectRatioComboBox->clear();
        _aspectRatioComboBox->addItem(tr("Square"), ZoomSettings::SQUARE);
        _aspectRatioComboBox->addItem(tr("NTSC"), ZoomSettings::NTSC);
        _aspectRatioComboBox->addItem(tr("PAL"), ZoomSettings::PAL);

        connect(_aspectRatioComboBox, qOverload<int>(&QComboBox::activated),
                this, &ZoomSettingsUi::onAspectRatioComboBoxActivated);
    }
    {
        _zoomInAction->setText(tr("Zoom In"));
        _zoomInAction->setIcon(QIcon(":/icons/zoom-in.svg"));
        _zoomInAction->setShortcut(Qt::CTRL + Qt::Key_Plus);

        connect(_zoomInAction, &QAction::triggered,
                this, &ZoomSettingsUi::onZoomInTriggered);
    }
    {
        _zoomOutAction->setText(tr("Zoom Out"));
        _zoomOutAction->setIcon(QIcon(":/icons/zoom-out.svg"));
        _zoomOutAction->setShortcut(Qt::CTRL + Qt::Key_Minus);

        connect(_zoomOutAction, &QAction::triggered,
                this, &ZoomSettingsUi::onZoomOutTriggered);
    }
}

void ZoomSettingsUi::setEnabled(bool enabled)
{
    if (_settings == nullptr) {
        enabled = false;
    }

    _zoomMenu->setEnabled(enabled);
    _aspectRatioMenu->setEnabled(enabled);
    _zoomComboBox->setEnabled(enabled);
    _aspectRatioComboBox->setEnabled(enabled);
    _zoomInAction->setEnabled(enabled);
    _zoomOutAction->setEnabled(enabled);
}

void ZoomSettingsUi::setZoomSettings(ZoomSettings* settings)
{
    if (_settings != settings) {
        if (_settings) {
            _settings->disconnect(this);
        }
        _settings = settings;

        setEnabled(settings != nullptr);
        updateZoomGui();
        updateAspectRatioGui();

        if (settings) {
            connect(settings, &ZoomSettings::zoomChanged,
                    this, &ZoomSettingsUi::updateZoomGui);
            connect(settings, &ZoomSettings::aspectRatioChanged,
                    this, &ZoomSettingsUi::updateAspectRatioGui);
        }
    }
}

void ZoomSettingsUi::populateMenu(QMenu* menu)
{
    menu->addMenu(_zoomMenu);
    menu->addMenu(_aspectRatioMenu);

    menu->addAction(_zoomInAction);
    menu->addAction(_zoomOutAction);
}

void ZoomSettingsUi::updateZoomGui()
{
    if (_settings) {
        auto zoom = _settings->zoom();

        int index = _zoomComboBox->findData(zoom);
        _zoomComboBox->setCurrentIndex(index);
        if (index == -1) {
            _zoomComboBox->setEditText(_settings->zoomString());
        }

        for (QAction* a : _zoomMenu->actions()) {
            a->setChecked(a->data() == zoom);
        }
    }
    else {
        _zoomComboBox->setCurrentIndex(-1);

        for (QAction* a : _zoomMenu->actions()) {
            a->setChecked(false);
        }
    }
}

void ZoomSettingsUi::updateAspectRatioGui()
{
    if (_settings) {
        auto aspectRatio = _settings->aspectRatio();

        int index = _aspectRatioComboBox->findData(aspectRatio);
        _aspectRatioComboBox->setCurrentIndex(index);

        for (QAction* a : _aspectRatioMenu->actions()) {
            if (a->data() == aspectRatio) {
                a->setChecked(true);
                break;
            }
        }
    }
    else {
        _aspectRatioComboBox->setCurrentIndex(-1);

        for (QAction* a : _aspectRatioMenu->actions()) {
            a->setChecked(false);
        }
    }
}

void ZoomSettingsUi::onZoomComboBoxActivated(int index)
{
    if (_settings == nullptr) {
        return;
    }

    qreal z = _zoomComboBox->itemData(index).toDouble();
    if (z > 0) {
        _settings->setZoom(z);
    }
    else {
        _settings->resetZoom();
    }
}

void ZoomSettingsUi::onZoomComboBoxEdited()
{
    if (_settings == nullptr) {
        return;
    }

    _settings->setZoom(_zoomComboBox->currentText());
    updateZoomGui();
}

void ZoomSettingsUi::onZoomMenuTriggered(QAction* action)
{
    if (_settings == nullptr || action == nullptr) {
        return;
    }

    qreal z = action->data().toDouble();
    if (z > 0) {
        _settings->setZoom(z);
    }
    else {
        _settings->resetZoom();
    }
}

void ZoomSettingsUi::onAspectRatioComboBoxActivated(int index)
{
    if (_settings == nullptr) {
        return;
    }

    int a = _aspectRatioComboBox->itemData(index).toInt();
    _settings->setAspectRatio(ZoomSettings::AspectRatio(a));
}

void ZoomSettingsUi::onAspectRatioMenuTriggered(QAction* action)
{
    if (_settings == nullptr || action == nullptr) {
        return;
    }

    int a = action->data().toInt();
    _settings->setAspectRatio(ZoomSettings::AspectRatio(a));
}

void ZoomSettingsUi::onZoomInTriggered()
{
    if (_settings) {
        _settings->zoomIn();
    }
}

void ZoomSettingsUi::onZoomOutTriggered()
{
    if (_settings) {
        _settings->zoomOut();
    }
}
