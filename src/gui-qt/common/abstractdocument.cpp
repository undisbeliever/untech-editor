/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractdocument.h"

#include <QFileInfo>
#include <QMessageBox>

using namespace UnTech::GuiQt;

AbstractDocument::AbstractDocument(QObject* parent)
    : QObject(parent)
    , _filename()
    , _undoStack(new QUndoStack(this))
{
}

bool AbstractDocument::saveDocument(const QString& filename)
{
    QString absFilename = QFileInfo(filename).absoluteFilePath();
    bool success = false;

    try {
        success = saveDocumentFile(absFilename);
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error Saving File"), ex.what());
        success = false;
    }

    if (success) {
        if (_filename != absFilename) {
            _filename = absFilename;
            filenameChanged();
        }
        _undoStack->setClean();
    }

    return success;
}

bool AbstractDocument::loadDocument(const QString& filename)
{
    QString absFilename = QFileInfo(filename).absoluteFilePath();
    bool success = false;

    try {
        success = loadDocumentFile(absFilename);
    }
    catch (const std::exception& ex) {
        QMessageBox::critical(nullptr, tr("Error opening File"), ex.what());
        success = false;
    }

    if (success) {
        if (_filename != absFilename) {
            _filename = absFilename;
            filenameChanged();
        }
        _undoStack->clear();
    }

    return success;
}
