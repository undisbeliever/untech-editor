/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "document.h"
#include "framelistmodel.h"
#include "selection.h"

#include <QMessageBox>

using namespace UnTech::GuiQt::MetaSprite::SpriteImporter;

Document::Document(QObject* parent)
    : AbstractMsDocument(parent)
    , _frameSet(std::make_unique<SI::FrameSet>())
    , _selection(new Selection(this))
    , _frameListModel(new FrameListModel(this))
{
    initModels();
}

void Document::initModels()
{
    _selection->setDocument(this);
    _frameListModel->setDocument(this);

    AbstractMsDocument::initModels();
}

const QString& Document::fileFilter() const
{
    static const QString FILTER = QString::fromUtf8(
        "SpriteImporter FrameSet (*.utsi);;All Files (*)");

    return FILTER;
}

const QString& Document::defaultFileExtension() const
{
    static const QString EXTENSION = QString::fromUtf8("utsi");
    return EXTENSION;
}

bool Document::saveDocumentFile(const QString& filename)
{
    try {
        SI::saveFrameSet(*_frameSet, filename.toUtf8().data());
        return true;
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Saving File"), ex.what());
        return false;
    }
}

bool Document::loadDocumentFile(const QString& filename)
{
    try {
        auto fs = SI::loadFrameSet(filename.toUtf8().data());
        if (fs) {
            _frameSet = std::move(fs);
            initModels();
            return true;
        }
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Opening File"), ex.what());
    }
    return false;
}
