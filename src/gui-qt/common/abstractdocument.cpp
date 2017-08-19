/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "abstractdocument.h"

using namespace UnTech::GuiQt;

AbstractDocument::AbstractDocument(QObject* parent)
    : QObject(parent)
    , _filename()
    , _undoStack(new QUndoStack(this))
{
}

bool AbstractDocument::saveDocument(const QString& filename)
{
    bool success = saveDocumentFile(filename);

    if (success) {
        if (_filename != filename) {
            _filename = filename;
            filenameChanged();
        }
        _undoStack->setClean();
    }

    return success;
}

bool AbstractDocument::loadDocument(const QString& filename)
{
    bool success = loadDocumentFile(filename);

    if (success) {
        if (_filename != filename) {
            _filename = filename;
            filenameChanged();
        }
        _undoStack->clear();
    }

    return success;
}
