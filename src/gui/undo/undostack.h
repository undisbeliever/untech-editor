#ifndef _UNTECH_GUI_UNDO_UNDOSTACK_H_
#define _UNTECH_GUI_UNDO_UNDOSTACK_H_

#include <list>
#include <memory>
#include <glibmm/i18n.h>
#include <glibmm/ustring.h>
#include <sigc++/signal.h>

namespace UnTech {
namespace Undo {

/**
 * A virtual class whose subclasses will contain enough state to
 * undo and redo all the given action.
 *
 * The subclass is not responsible for preforming the action, that will
 * be the responsibility of the function that initiates the subclass
 * (see the actionhelper.h macros)
 */
class Action {
public:
    virtual ~Action() = default;

    /** Called by UndoStack when user presses undo */
    virtual void undo() = 0;

    /** Called by UndoStack when user presses redo */
    virtual void redo() = 0;

    virtual const Glib::ustring& message() const = 0;
};

/**
 * A simple undo stack that holds the Action subclasses, ready for the
 * undo and redo functions.
 */
class UndoStack {
    const unsigned STACK_LIMIT = 100;

public:
    UndoStack() = default;
    ~UndoStack() = default;

    void add_undo(std::unique_ptr<Action> action);

    void undo();
    void redo();

    void clear();

    inline bool canUndo() const { return !_undoStack.empty(); }
    inline bool canRedo() const { return !_redoStack.empty(); }

    const Glib::ustring& getUndoMessage() const;
    const Glib::ustring& getRedoMessage() const;

    sigc::signal<void> signal_stackChanged;

private:
    // using list instead of stack so I can delete from the end.
    std::list<std::unique_ptr<Action>> _undoStack;
    std::list<std::unique_ptr<Action>> _redoStack;
};
}
}

#endif
