/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2018, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include "models/common/idstring.h"
#include <QRegExp>
#include <QValidator>

namespace UnTech {
namespace GuiQt {

class IdstringValidator : public QValidator {
    Q_OBJECT

    static const QRegExp INVALID_CHAR_REGEX;
    static const QString INVALID_REPLACEMENT;

public:
    explicit IdstringValidator(QObject* parent = 0);
    ~IdstringValidator() = default;

    virtual void fixup(QString& input) const override;
    virtual State validate(QString& input, int& pos) const override;
};
}
}
