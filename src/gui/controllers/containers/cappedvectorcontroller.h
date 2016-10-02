#pragma once

#include "models/common/capped_vector.h"
#include "models/common/optional.h"
#include <algorithm>
#include <cassert>
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

    CappedVectorController(ParentT& parent)
        : _parent(parent)
    {
        parent.signal_selectedChanged().connect(sigc::mem_fun(
            *this, &CappedVectorController::reloadList));

        this->_signal_listChanged.connect(sigc::mem_fun(
            *this, &CappedVectorController::validateSelection));
    }

    ParentT& parent() { return _parent; }
    const ParentT& parent() const { return _parent; }

    const list_type* list() const { return _list; }

    // Reloads the list from the parent
    void reloadList()
    {
        list_type* l = editable_listFromParent();
        if (l != _list) {
            _selectedIndex = ~0;
            _hasSelected = false;
            _list = l;

            _signal_selectedChanged.emit();
            _signal_listChanged.emit();
            _signal_anyChanged.emit();
        }
        else {
            validateSelection();
        }
    }

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

    void selectNone()
    {
        if (_hasSelected) {
            _selectedIndex = ~0;
            _hasSelected = false;

            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    void selectIndex(size_t index)
    {
        if (_list && index < _list->size()) {
            _selectedIndex = index;
            _hasSelected = true;

            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
        else {
            selectNone();
        }
    }

    void validateSelection()
    {
        if (_list == nullptr || _selectedIndex >= _list->size()) {
            selectNone();
        }
    }

    bool canCreate() const
    {
        return _list && _list->can_insert();
    }
    void create()
    {
        if (canCreate()) {
            // ::TODO undo engine::

            _list->emplace_back();

            _selectedIndex = _list->size() - 1;
            _hasSelected = true;

            _signal_listChanged.emit();
            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    bool canCloneSelected() const
    {
        return _list && _hasSelected && _list->can_insert();
    }
    void cloneSelected()
    {
        if (canCloneSelected()) {
            // ::TODO undo engine::

            _list->emplace_back(_list->at(_selectedIndex));

            _selectedIndex = _list->size() - 1;
            _hasSelected = true;

            _signal_listChanged.emit();
            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    bool canRemoveSelected() const
    {
        return _list && _hasSelected;
    }
    void removeSelected()
    {
        if (canRemoveSelected()) {
            // ::TODO undo engine::

            assert(_selectedIndex < _list->size());

            auto it = _list->begin() + _selectedIndex;
            _list->erase(it);

            selectNone();
            _signal_listChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    bool canMoveSelectedUp() const
    {
        return _list && _hasSelected && _selectedIndex > 0;
    }
    void moveSelectedUp()
    {
        if (canMoveSelectedUp()) {
            // ::TODO undo engine::

            assert(_selectedIndex < _list->size());

            auto it = _list->begin() + _selectedIndex;
            std::iter_swap(it, it - 1);

            _selectedIndex = _selectedIndex - 1;
            _hasSelected = true;

            _signal_listChanged.emit();
            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    bool canMoveSelectedDown() const
    {
        return _list && _hasSelected && _selectedIndex < _list->size() - 1;
    }
    void moveSelectedDown()
    {
        if (canMoveSelectedDown()) {
            // ::TODO undo engine::

            assert(_selectedIndex + 1 < _list->size());

            auto it = _list->begin() + _selectedIndex;
            std::iter_swap(it, it + 1);

            _selectedIndex = _selectedIndex + 1;
            _hasSelected = true;

            _signal_listChanged.emit();
            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }

    auto& signal_anyChanged() { return _signal_anyChanged; }
    auto& signal_dataChanged() { return _signal_dataChanged; }
    auto& signal_listChanged() { return _signal_listChanged; }
    auto& signal_selectedChanged() { return _signal_selectedChanged; }

protected:
    virtual list_type* editable_listFromParent() = 0;

    // may be NULL if nothing is selected
    element_type* editable_selected()
    {
        if (_list && _hasSelected) {
            return &_list->at(_selectedIndex);
        }
        else {
            return nullptr;
        }
    }

    void edit_selected(std::function<void(element_type&)> const& fun)
    {
        if (_list && _hasSelected) {
            // ::TODO undo engine::

            assert(_selectedIndex < _list->size());

            fun(_list->at(_selectedIndex));
        }

        _signal_dataChanged.emit();
        _signal_anyChanged.emit();
    }

protected:
    ParentT& _parent;
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
