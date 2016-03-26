#ifndef _UNTECH_GUI_UNDO_UNDODOCUMENT_H_
#define _UNTECH_GUI_UNDO_UNDODOCUMENT_H_

#include "undostack.h"

namespace UnTech {
namespace Undo {

/*
 * A place holder class for an UndoStack.
 */
class UndoDocument {
public:
    UndoDocument() = default;
    virtual ~UndoDocument() = default;

    UndoStack& undoStack() { return _undoStack; }

private:
    UndoStack _undoStack;
};
}
}
#endif
