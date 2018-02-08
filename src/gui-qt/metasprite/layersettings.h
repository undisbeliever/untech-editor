/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QAction>
#include <QActionGroup>
#include <QMenu>
#include <QObject>

namespace UnTech {
namespace GuiQt {
namespace MetaSprite {

class LayerSettings : public QObject {
    Q_OBJECT

public:
    explicit LayerSettings(QObject* parent = 0);
    ~LayerSettings() = default;

    bool showOrigin() const { return _showOrigin; }
    bool showTileHitbox() const { return _showTileHitbox; }
    bool showFrameObjects() const { return _showFrameObjects; }
    bool showActionPoints() const { return _showActionPoints; }
    bool showEntityHitboxes() const { return _showEntityHitboxes; }

    void populateMenu(QMenu* menu);

public slots:
    void showAll();
    void setShowOrigin(bool showOrigin);
    void setShowTileHitbox(bool showTileHitbox);
    void setShowFrameObjects(bool showFrameObjects);
    void setShowActionPoints(bool showActionPoints);
    void setShowEntityHitboxes(bool showEntityHitboxes);

signals:
    void layerSettingsChanged();

private:
    bool _showOrigin;
    bool _showTileHitbox;
    bool _showFrameObjects;
    bool _showActionPoints;
    bool _showEntityHitboxes;

    QAction* _showAllAction;
    QAction* _showOriginAction;
    QAction* _showTileHitboxAction;
    QAction* _showFrameObjectsAction;
    QAction* _showActionPointsAction;
    QAction* _showEntityHitboxesAction;
};
}
}
}
