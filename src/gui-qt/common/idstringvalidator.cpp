/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "idstringvalidator.h"

using namespace UnTech::GuiQt;

const QRegExp IdstringValidator::INVALID_CHAR_REGEX = QRegExp("[^_0-9A-Za-z]");
const QString IdstringValidator::INVALID_REPLACEMENT = QString('_');

IdstringValidator::IdstringValidator(QObject* parent)
    : QValidator(parent)
{
}

void IdstringValidator::fixup(QString& input) const
{
    input.replace(INVALID_CHAR_REGEX, INVALID_REPLACEMENT);
}

QValidator::State IdstringValidator::validate(QString& input, int&) const
{
    if (input.isEmpty()) {
        return Intermediate;
    }

    input.replace(INVALID_CHAR_REGEX, INVALID_REPLACEMENT);
    return Acceptable;
}
