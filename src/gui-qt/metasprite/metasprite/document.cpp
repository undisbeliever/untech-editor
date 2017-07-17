/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "framecontentsmodel.h"
#include "framelistmodel.h"
#include "palettesmodel.h"
#include "selection.h"

#include <QMessageBox>

using namespace UnTech::GuiQt::MetaSprite::MetaSprite;

const char* Document::FILE_FILTER = "MetaSprite FrameSet (*.utms);;All Files (*)";

Document::Document(QObject* parent)
    : Document(std::make_unique<MS::FrameSet>(), QString(), parent)
{
}

Document::Document(std::unique_ptr<MS::FrameSet> frameSet,
                   const QString& filename, QObject* parent)
    : AbstractDocument(parent)
    , _frameSet(std::move(frameSet))
    , _selection(new Selection(this))
    , _frameListModel(new FrameListModel(this))
    , _frameContentsModel(new FrameContentsModel(this))
    , _palettesModel(new PalettesModel(this))
{
    Q_ASSERT(_frameSet != nullptr);

    _filename = filename;

    _selection->setDocument(this);
    _frameListModel->setDocument(this);
    _frameContentsModel->setDocument(this);
    _palettesModel->setDocument(this);

    initModels();
}

std::unique_ptr<Document> Document::loadDocument(const QString& filename)
{
    try {
        return std::make_unique<Document>(
            MS::loadFrameSet(filename.toUtf8().data()), filename);
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Opening File"), ex.what());

        return nullptr;
    }
}

bool Document::saveDocument(const QString& filename)
{
    try {
        MS::saveFrameSet(*_frameSet, filename.toUtf8().data());
        _filename = filename;
        _undoStack->setClean();

        return true;
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Saving File"), ex.what());

        return false;
    }
}
