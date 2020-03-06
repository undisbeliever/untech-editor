/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "editorwidget.h"
#include "backgroundimagegraphicsitem.h"
#include "managers.h"
#include "resourceitem.h"
#include "gui-qt/common/graphics/zoomsettingsmanager.h"
#include "gui-qt/common/properties/propertylistview.h"
#include "gui-qt/project.h"
#include "gui-qt/resources/invalidimageerror.h"
#include "models/project/project-data.h"
#include "models/resources/invalid-image-error.h"

#include "gui-qt/resources/background-image/editorwidget.ui.h"

using namespace UnTech::GuiQt::Resources::BackgroundImage;

EditorWidget::EditorWidget(ZoomSettingsManager* zoomManager, QWidget* parent)
    : AbstractEditorWidget(parent)
    , _ui(new Ui::EditorWidget)
    , _propertyManager(new BackgroundImagePropertyManager(this))
    , _graphicsScene(new QGraphicsScene(this))
    , _biGraphicsItem(new BackgroundImageGraphicsItem())
    , _pixmapItem(new QGraphicsPixmapItem())
    , _resourceItem(nullptr)
    , _errorGraphicsItem(nullptr)
    , _animationTimer()
{
    _ui->setupUi(this);

    _animationTimer.setRegionCombo(_ui->region);
    _animationTimer.setPlayButton(_ui->playButton);

    _ui->graphicsView->setZoomSettings(zoomManager->get("background-image"));
    _ui->graphicsView->setScene(_graphicsScene);

    _biGraphicsItem->setTransparentTiles(false);

    _graphicsScene->addItem(_biGraphicsItem);
    _graphicsScene->addItem(_pixmapItem);

    addDockWidget(Qt::RightDockWidgetArea,
                  createPropertyDockWidget(_propertyManager, tr("Properties"), QStringLiteral("Properties_Dock")));

    connect(&_animationTimer, &AnimationTimer::animationStarted,
            _biGraphicsItem, &BackgroundImageGraphicsItem::resetPaletteFrame);
    connect(&_animationTimer, &AnimationTimer::animationFrameAdvance,
            _biGraphicsItem, &BackgroundImageGraphicsItem::nextPaletteFrame);

    connect(_ui->nextButton, &QToolButton::clicked,
            &_animationTimer, &AnimationTimer::stopTimer);
    connect(_ui->nextButton, &QToolButton::clicked,
            _biGraphicsItem, &BackgroundImageGraphicsItem::nextPaletteFrame);

    connect(_biGraphicsItem, &BackgroundImageGraphicsItem::paletteFrameChanged,
            this, &EditorWidget::updateAnimationFrameLabelText);

    setEnabled(false);
}

EditorWidget::~EditorWidget() = default;

QString EditorWidget::windowStateName() const
{
    return QStringLiteral("BackgroundImage");
}

UnTech::GuiQt::ZoomSettings* EditorWidget::zoomSettings() const
{
    return _ui->graphicsView->zoomSettings();
}

bool EditorWidget::setResourceItem(AbstractResourceItem* abstractItem)
{
    auto* item = qobject_cast<ResourceItem*>(abstractItem);

    if (_resourceItem == item) {
        return item != nullptr;
    }

    if (_resourceItem) {
        _resourceItem->disconnect(this);
    }
    _resourceItem = item;

    _propertyManager->setResourceItem(item);
    _animationTimer.stopTimer();

    if (_resourceItem) {
        connect(_resourceItem, &ResourceItem::resourceComplied,
                this, &EditorWidget::onResourceComplied);

        connect(_resourceItem, &ResourceItem::externalFilesModified,
                this, &EditorWidget::updatePixmap);

        connect(_resourceItem, &ResourceItem::errorListChanged,
                this, &EditorWidget::updateInvalidTiles);
    }

    onResourceComplied();
    updatePixmap();
    updateInvalidTiles();

    setEnabled(item != nullptr);

    return item != nullptr;
}

void EditorWidget::onResourceComplied()
{
    if (_resourceItem == nullptr) {
        _biGraphicsItem->removePixmaps();
        _biGraphicsItem->setVisible(false);
        _animationTimer.setEnabled(false);
        _ui->animationFrameLabel->clear();

        return;
    }

    const unsigned biIndex = _resourceItem->index();

    auto& projectData = _resourceItem->project()->projectData();
    auto biData = projectData.backgroundImages().at(biIndex);

    auto palIndex = biData ? std::optional(biData->conversionPaletteIndex) : std::nullopt;
    auto palData = projectData.palettes().at(palIndex);

    if (biData && palData) {
        _biGraphicsItem->drawPixmaps(*biData, *palData);
        _animationTimer.setAnimationDelay(palData->animationDelay);
        _animationTimer.setEnabled(palData->nAnimations() > 0);
    }
    else {
        _biGraphicsItem->removePixmaps();
        _animationTimer.setEnabled(false);
        _animationTimer.setEnabled(false);
    }

    updateAnimationFrameLabelText();

    bool graphicsOk = _biGraphicsItem->hasPixmaps();
    _biGraphicsItem->setVisible(graphicsOk);
    _pixmapItem->setVisible(graphicsOk == false);
}

void EditorWidget::updateAnimationFrameLabelText()
{
    if (_animationTimer.isEnabled()) {
        _ui->animationFrameLabel->setText(
            tr("Palette Frame %1").arg(_biGraphicsItem->paletteFrame()));
    }
    else {
        _ui->animationFrameLabel->clear();
    }
}

void EditorWidget::updatePixmap()
{
    if (_resourceItem) {
        const std::string& fn = _resourceItem->backgroundImageInput().imageFilename;
        _pixmapItem->setPixmap(QPixmap(QString::fromStdString(fn)));
    }
    else {
        _pixmapItem->setPixmap(QPixmap());
    }
}

void EditorWidget::updateInvalidTiles()
{
    if (_errorGraphicsItem) {
        delete _errorGraphicsItem;
    }
    _errorGraphicsItem = nullptr;

    if (_resourceItem == nullptr) {
        return;
    }

    // Create a new group for the errors
    _errorGraphicsItem = new QGraphicsItemGroup();
    _errorGraphicsItem->setZValue(10);

    for (const auto& errorItem : _resourceItem->errorList().list()) {
        if (auto* imgErr = dynamic_cast<const RES::InvalidImageError*>(errorItem.specialized.get())) {
            createGraphicsItemsForImageError(*imgErr, _errorGraphicsItem);
        }
    }

    _graphicsScene->addItem(_errorGraphicsItem);
}
