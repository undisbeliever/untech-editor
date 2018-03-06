/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <QLineEdit>
#include <QToolButton>
#include <QWidget>

namespace UnTech {
namespace GuiQt {

class FilenameInputWidget : public QWidget {
    Q_OBJECT

public:
    FilenameInputWidget(QWidget* parent = nullptr);
    ~FilenameInputWidget() = default;

    void setFrame(bool);

    QString filename() const { return _lineEdit->text(); }
    void setFilename(const QString& filename);
    void clear();

    const QString& dialogTitle() const { return _dialogTitle; }
    const QString& dialogFilter() const { return _dialogFilter; }
    bool isBlankFilenameAccepted() const { return _blankFilenameAccepted; }

    void setDialogTitle(const QString& title);
    void setDialogFilter(const QString& filter);
    void setBlankFilenameAccepted(bool blankFilenameAccepted);

public slots:
    void showDialog();

signals:
    void filenameChanged(const QString& filename);
    void fileSelected(const QString& filename);

private:
    QLineEdit* _lineEdit;
    QToolButton* _button;

    QString _dialogTitle;
    QString _dialogFilter;
    bool _blankFilenameAccepted;
};
}
}
