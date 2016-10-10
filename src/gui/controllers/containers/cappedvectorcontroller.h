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
public:
    using element_type = ElementT;
    using list_type = ListT;

    const static element_type BLANK_T;

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
    virtual list_type* editable_listFromParent() = 0;

    // called when element is created but before signals are emitted
    // can edit element_type
    virtual void onCreate(element_type&);

    // may be NULL if nothing is selected
    element_type* editable_selected();

    void edit_selected(std::function<void(element_type&)> const& fun);

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
