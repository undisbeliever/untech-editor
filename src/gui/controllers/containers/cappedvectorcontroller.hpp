#include "cappedvectorcontroller.h"

#include <algorithm>
#include <cassert>

namespace UnTech {
namespace Controller {

template <class ListT, class ParentT>
ListT& listFromParent(ParentT&);

template <typename ET, class LT, class PT>
class CappedVectorController<ET, LT, PT>::MementoUndoAction : public Undo::Action {
public:
    MementoUndoAction() = delete;
    MementoUndoAction(CappedVectorController& controller, const ET& value)
        : _controller(controller)
        , _ref(controller.undoRefForSelected())
        , _oldValue(value)
        , _newValue()
    {
    }
    virtual ~MementoUndoAction() override = default;

    virtual void undo() override
    {
        ET* element = CappedVectorController::elementFromUndoRef(_ref);
        if (element) {
            *element = _oldValue;

            _controller._signal_dataChanged.emit();
            _controller._signal_anyChanged.emit();
        }
    }

    virtual void redo() override
    {
        ET* element = CappedVectorController::elementFromUndoRef(_ref);
        if (element) {
            *element = _newValue;

            _controller._signal_dataChanged.emit();
            _controller._signal_anyChanged.emit();
        }
    }

    virtual const std::string& message() const override
    {
        // ::TODO undo message::
        static const std::string s = "edit_selected";
        return s;
    }

    void setNewValue(const ET& value) { _newValue = value; }

private:
    CappedVectorController& _controller;
    UndoRef _ref;
    const ET _oldValue;
    ET _newValue;
};

template <typename ElementT, class ListT, class ParentT>
const ElementT CappedVectorController<ElementT, ListT, ParentT>::BLANK_T = ElementT();

template <typename ET, class LT, class ParentT>
CappedVectorController<ET, LT, ParentT>::CappedVectorController(ParentT& parent)
    : _parent(parent)
    , _baseController(parent.baseController())
{
    parent.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &CappedVectorController::reloadList));

    this->_signal_listChanged.connect(sigc::mem_fun(
        *this, &CappedVectorController::validateSelection));
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::reloadList()
{
    typename PT::element_type* p = _parent.editable_selected();

    list_type* l = nullptr;
    if (p) {
        l = &listFromParent<LT, typename PT::element_type>(*p);
    }

    if (l != _list || _list == nullptr) {
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
        if (index != _selectedIndex) {
            _selectedIndex = index;
            _hasSelected = true;

            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
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
        onCreate(_list->back());

        _selectedIndex = _list->size() - 1;
        _hasSelected = true;

        _signal_listChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();
    }
}

template <typename ET, class LT, class PT>
void CappedVectorController<ET, LT, PT>::onCreate(ET&)
{
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
typename CappedVectorController<ET, LT, PT>::UndoRef
CappedVectorController<ET, LT, PT>::undoRefForSelected() const
{
    if (_list && hasSelected()) {
        return {
            _parent.undoRefForSelected(),
            _selectedIndex
        };
    }
    else {
        throw std::logic_error("No element selected");
    }
}

template <typename ET, class LT, class PT>
ET* CappedVectorController<ET, LT, PT>::elementFromUndoRef(const UndoRef& ref)
{
    auto* p = PT::elementFromUndoRef(ref.parent);
    if (p) {
        auto& list = listFromParent<LT, typename PT::element_type>(*p);
        return &list.at(ref.index);
    }

    return nullptr;
}

template <typename ET, class LT, class PT>
template <class UndoActionT>
void CappedVectorController<ET, LT, PT>::edit_selected(
    std::function<bool(const ET&)> const& validate,
    std::function<void(ET&)> const& fun)
{
    if (_list && _hasSelected) {
        assert(_selectedIndex < _list->size());
        auto& value = _list->at(_selectedIndex);

        if (validate(value)) {
            auto action = std::make_unique<UndoActionT>(*this, value);

            fun(value);

            action->setNewValue(value);
            _baseController.undoStack().add_undo(std::move(action));
        }
    }

    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}
}
}
