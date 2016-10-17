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
