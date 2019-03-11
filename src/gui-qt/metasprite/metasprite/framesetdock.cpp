/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "framesetdock.h"
#include "accessors.h"
#include "document.h"
#include "managers.h"
#include "gui-qt/common/idstringvalidator.h"
#include "gui-qt/metasprite/metasprite/framesetdock.ui.h"

#include <QMenu>
#include <QMessageBox>

using namespace UnTech::GuiQt;
using namespace UnTech::GuiQt::MetaSprite::MetaSprite;
using TilesetType = UnTech::MetaSprite::TilesetType;

FrameSetDock::FrameSetDock(QWidget* parent)
    : QDockWidget(parent)
    , _ui(new Ui::FrameSetDock)
    , _document(nullptr)
    , _frameSetManager(new FrameSetManager(this))
{
    _ui->setupUi(this);

    _ui->frameSetProperties->setPropertyManager(_frameSetManager);

    _ui->frameList->namedListActions().populate(_ui->frameListButtons);

    setEnabled(false);
}

FrameSetDock::~FrameSetDock() = default;

void FrameSetDock::setDocument(Document* document)
{
    if (_document == document) {
        return;
    }

    if (_document != nullptr) {
        _document->disconnect(this);
    }
    _document = document;

    setEnabled(_document != nullptr);

    FrameList* frameList = _document ? _document->frameList() : nullptr;

    _frameSetManager->setDocument(_document);
    _ui->frameList->setAccessor(frameList);
}

const Accessor::NamedListActions& FrameSetDock::frameActions() const
{
    return _ui->frameList->namedListActions();
}

Accessor::NamedListModel* FrameSetDock::frameListModel() const
{
    return _ui->frameList->namedListModel();
}

void FrameSetDock::populateMenu(QMenu* menu)
{
    _ui->frameList->namedListActions().populate(menu);
    // :: TODO add toggle tileset hitbox here::
}
