/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#include "controllerinterface.h"

using namespace UnTech::View;

ControllerInterface::ControllerInterface(wxFrame* frame)
    : _frame(frame)
{
}

void ControllerInterface::showError(const char* error, const std::exception& ex)
{
    auto* dialog = new wxMessageDialog(_frame,
                                       error, "Error",
                                       wxOK | wxCENTRE | wxICON_ERROR);

    dialog->SetExtendedMessage(ex.what());

    dialog->ShowModal();
}
