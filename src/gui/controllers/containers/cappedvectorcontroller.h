#pragma once

#include "gui/controllers/basecontroller.h"
#include "models/common/capped_vector.h"
#include "models/common/optional.h"
#include <functional>
#include <sigc++/signal.h>

namespace UnTech {
namespace Controller {

template <typename ElementT, class ListT, class ParentT>
class CappedVectorController {
    template <typename, class, class>
    friend class CappedVectorController;
    template <class, class>
    friend class IdMapController;

public:
    using element_type = ElementT;
    using list_type = ListT;

private:
    struct UndoRef {
        const typename ParentT::UndoRef parent;
        const size_t index;

        bool operator==(const UndoRef& o) const { return parent == o.parent && index == o.index; }
    };
    class MementoUndoAction;
    class CreateUndoAction;
    class RemoveUndoAction;
    class MoveUndoAction;

    const static element_type BLANK_T;

public:
    CappedVectorController(const CappedVectorController&) = delete;
    virtual ~CappedVectorController() = default;

    CappedVectorController(ParentT& parent);

    ParentT& parent() { return _parent; }
    const ParentT& parent() const { return _parent; }

    BaseController& baseController() { return _baseController; }
    const BaseController& baseController() const { return _baseController; }

    const list_type* list() const { return _list; }

    // Reloads the list from the parent
    void reloadList();

    optional<size_t> selectedIndex() const
    {
        if (_hasSelected) {
            return _selectedIndex;
        }
        else {
            return optional<size_t>();
        }
    }

    const element_type& selected() const
    {
        if (_list && _hasSelected) {
            return _list->at(_selectedIndex);
        }
        else {
            return BLANK_T;
        }
    }

    bool hasSelected() const { return _hasSelected; }

    void selectNone();
    void selectIndex(size_t index);

    void validateSelection();

    bool canCreate() const;
    void create();

    bool canCloneSelected() const;
    void cloneSelected();

    bool canRemoveSelected() const;
    void removeSelected();

    bool canMoveSelectedUp() const;
    void moveSelectedUp();

    bool canMoveSelectedDown() const;
    void moveSelectedDown();

    auto& signal_anyChanged() { return _signal_anyChanged; }
    auto& signal_dataChanged() { return _signal_dataChanged; }
    auto& signal_listChanged() { return _signal_listChanged; }
    auto& signal_selectedChanged() { return _signal_selectedChanged; }

protected:
    // called when element is created but before signals are emitted
    // can edit element_type
    virtual void onCreate(element_type&);

    // may be NULL if nothing is selected
    element_type* editable_selected();

    // will only create an UndoAction and call fun if validate returns true.
    // actionType MUST EXIST forever (preferably as a const static)
    template <class UndoActionT = MementoUndoAction>
    void edit_selected(
        const Undo::ActionType* actionType,
        std::function<bool(const element_type&)> const& validate,
        std::function<void(element_type&)> const& fun);

    // throws exception if nothing is selected
    UndoRef undoRefForSelected() const;

    // throws exception if reference is invalid
    static element_type& elementFromUndoRef(const UndoRef& ref);

protected:
    ParentT& _parent;
    BaseController& _baseController;

    list_type* _list;
    size_t _selectedIndex;
    bool _hasSelected;

    sigc::signal<void> _signal_anyChanged;
    sigc::signal<void> _signal_dataChanged;
    sigc::signal<void> _signal_listChanged;
    sigc::signal<void> _signal_selectedChanged;
};
}
}
