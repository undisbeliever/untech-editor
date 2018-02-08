/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QObject>
#include <QUndoStack>

namespace UnTech {
namespace GuiQt {

class AbstractDocument : public QObject {
    Q_OBJECT

public:
    static const char* FILE_FILTER;

public:
    explicit AbstractDocument(QObject* parent = nullptr);
    ~AbstractDocument() = default;

public:
    bool saveDocument(const QString& filename);
    bool loadDocument(const QString& filename);

    const QString& filename() const { return _filename; }
    QUndoStack* undoStack() const { return _undoStack; }

    virtual const QString& fileFilter() const = 0;
    virtual const QString& defaultFileExtension() const = 0;

protected:
    virtual bool saveDocumentFile(const QString& filename) = 0;
    virtual bool loadDocumentFile(const QString& filename) = 0;

signals:
    void filenameChanged();

protected:
    QString _filename;
    QUndoStack* _undoStack;
};
}
}
