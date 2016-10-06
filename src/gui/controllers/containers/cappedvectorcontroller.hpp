#include "cappedvectorcontroller.h"

#include <algorithm>
#include <cassert>

namespace UnTech {
namespace Controller {

template <typename ElementT, class ListT, class ParentT>
const ElementT CappedVectorController<ElementT, ListT, ParentT>::BLANK_T = ElementT();

template <typename ET, class LT, class ParentT>
CappedVectorController<ET, LT, ParentT>::CappedVectorController(ParentT& parent)
    : _parent(parent)
{
    parent.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &CappedVectorController::reloadList));

    this->_signal_listChanged.connect(sigc::mem_fun(
        *this, &CappedVectorController::validateSelection));
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::reloadList()
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

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::selectNone()
{
    if (_hasSelected) {
        _selectedIndex = ~0;
        _hasSelected = false;

        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();
    }
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::selectIndex(size_t index)
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

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::validateSelection()
{
    if (_list == nullptr || _selectedIndex >= _list->size()) {
        selectNone();
    }
}

template <typename ET, class LT, class PT>
bool CappedVectorController<ET, LT, PT>::canCreate() const
{
    return _list && _list->can_insert();
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::create()
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

template <typename ET, class LT, class PT>
bool CappedVectorController<ET, LT, PT>::canCloneSelected() const
{
    return _list && _hasSelected && _list->can_insert();
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::cloneSelected()
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

template <typename ET, class LT, class PT>
bool CappedVectorController<ET, LT, PT>::canRemoveSelected() const
{
    return _list && _hasSelected;
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::removeSelected()
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

template <typename ET, class LT, class PT>
bool CappedVectorController<ET, LT, PT>::canMoveSelectedUp() const
{
    return _list && _hasSelected && _selectedIndex > 0;
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::moveSelectedUp()
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

template <typename ET, class LT, class PT>
bool CappedVectorController<ET, LT, PT>::
    canMoveSelectedDown() const
{
    return _list && _hasSelected && _selectedIndex < _list->size() - 1;
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::moveSelectedDown()
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

template <typename ET, class LT, class PT>
ET* CappedVectorController<ET, LT, PT>::editable_selected()
{
    if (_list && _hasSelected) {
        return &_list->at(_selectedIndex);
    }
    else {
        return nullptr;
    }
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::edit_selected(std::function<void(ET&)> const& fun)
{
    if (_list && _hasSelected) {
        // ::TODO undo engine::

        assert(_selectedIndex < _list->size());

        fun(_list->at(_selectedIndex));
    }

    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}
}
}
