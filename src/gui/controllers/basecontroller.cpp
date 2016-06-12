#include "basecontroller.h"

#include "gui/widgets/common/errormessagedialog.h"

using namespace UnTech::Controller;

void BaseController::showError(const char* error, const std::exception& ex)
{
    UnTech::Widgets::showErrorMessage(_window, error, ex);
}

bool BaseController::dontMergeNextAction()
{
    _undoStack.dontMergeNextAction();
    return false;
}
