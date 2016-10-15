#include "cappedvectorcontroller.h"

#include "models/common/humantypename.h"
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
        elementFromUndoRef(_ref) = _oldValue;

        _controller._signal_dataChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual void redo() override
    {
        elementFromUndoRef(_ref) = _newValue;

        _controller._signal_dataChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual const std::string& message() const override
    {
        // ::SHOULDDO better undo message::
        static const std::string s = "Edit " + HumanTypeName<ET>::value;
        return s;
    }

    void setNewValue(const ET& value) { _newValue = value; }

private:
    CappedVectorController& _controller;
    const UndoRef _ref;
    const ET _oldValue;
    ET _newValue;
};

template <typename ET, class LT, class PT>
class CappedVectorController<ET, LT, PT>::CreateUndoAction : public Undo::Action {
public:
    CreateUndoAction() = delete;
    CreateUndoAction(CappedVectorController& controller, size_t index)
        : _controller(controller)
        , _ref(controller.parent().undoRefForSelected())
        , _index(index)
        , _element(_controller._list->at(index))
    {
    }
    virtual ~CreateUndoAction() override = default;

    virtual void undo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        list_type& list = listFromParent<LT, typename PT::element_type>(p);

        assert(_index < list.size());
        auto it = list.begin() + _index;
        list.erase(it);

        if (_controller._selectedIndex == _index) {
            _controller.selectNone();
        }

        _controller._signal_listChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual void redo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        list_type& list = listFromParent<LT, typename PT::element_type>(p);

        assert(_index <= list.size());
        auto it = list.begin() + _index;
        list.emplace(it, _element);

        _controller._signal_listChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual const std::string& message() const override
    {
        static const std::string s = "Create " + HumanTypeName<ET>::value;
        return s;
    }

private:
    CappedVectorController& _controller;
    const typename PT::UndoRef _ref;
    const size_t _index;
    const ET _element;
};

template <typename ET, class LT, class PT>
class CappedVectorController<ET, LT, PT>::RemoveUndoAction : public Undo::Action {
public:
    RemoveUndoAction() = delete;
    RemoveUndoAction(CappedVectorController& controller, size_t index)
        : _controller(controller)
        , _ref(controller.parent().undoRefForSelected())
        , _index(index)
        , _element(_controller._list->at(index))
    {
    }
    virtual ~RemoveUndoAction() override = default;

    virtual void undo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        list_type& list = listFromParent<LT, typename PT::element_type>(p);

        assert(_index <= list.size());
        auto it = list.begin() + _index;
        list.emplace(it, _element);

        _controller._signal_listChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual void redo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        list_type& list = listFromParent<LT, typename PT::element_type>(p);

        assert(_index < list.size());
        auto it = list.begin() + _index;
        list.erase(it);

        if (_controller._selectedIndex == _index) {
            _controller.selectNone();
        }

        _controller._signal_listChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual const std::string& message() const override
    {
        static const std::string s = "Remove " + HumanTypeName<ET>::value;
        return s;
    }

private:
    CappedVectorController& _controller;
    const typename PT::UndoRef _ref;
    const size_t _index;
    const ET _element;
};

template <typename ET, class LT, class PT>
class CappedVectorController<ET, LT, PT>::MoveUndoAction : public Undo::Action {
public:
    MoveUndoAction() = delete;
    MoveUndoAction(CappedVectorController& controller, size_t oldIndex, size_t newIndex)
        : _controller(controller)
        , _ref(controller.parent().undoRefForSelected())
        , _oldIndex(oldIndex)
        , _newIndex(newIndex)
    {
    }
    virtual ~MoveUndoAction() override = default;

    void doSwap(size_t oldIndex, size_t newIndex)
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        list_type& list = listFromParent<LT, typename PT::element_type>(p);

        assert(oldIndex < list.size() && newIndex < list.size());

        auto it1 = list.begin() + oldIndex;
        auto it2 = list.begin() + newIndex;

        std::iter_swap(it1, it2);

        _controller._signal_listChanged.emit();

        if (_controller._selectedIndex == oldIndex) {
            _controller._selectedIndex = newIndex;
            _controller._signal_selectedChanged.emit();
        }
        else if (_controller._selectedIndex == newIndex) {
            _controller._selectedIndex = oldIndex;
            _controller._signal_selectedChanged.emit();
        }

        _controller._signal_anyChanged.emit();
    }

    virtual void undo() override
    {
        doSwap(_newIndex, _oldIndex);
    }

    virtual void redo() override
    {
        doSwap(_oldIndex, _newIndex);
    }

    virtual const std::string& message() const override
    {
        static const std::string s = "Move " + HumanTypeName<ET>::value;
        return s;
    }

private:
    CappedVectorController& _controller;
    const typename PT::UndoRef _ref;
    const size_t _oldIndex, _newIndex;
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
        _list->emplace_back();
        onCreate(_list->back());

        _selectedIndex = _list->size() - 1;
        _hasSelected = true;

        _signal_listChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();

        _baseController.undoStack().add_undo(
            std::make_unique<CreateUndoAction>(*this, _selectedIndex));
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
        _list->emplace_back(_list->at(_selectedIndex));

        _selectedIndex = _list->size() - 1;
        _hasSelected = true;

        _signal_listChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();

        _baseController.undoStack().add_undo(
            std::make_unique<CreateUndoAction>(*this, _selectedIndex));
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
        auto action = std::make_unique<RemoveUndoAction>(*this, _selectedIndex);
        action->redo();

        _baseController.undoStack().add_undo(std::move(action));
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
        auto action = std::make_unique<MoveUndoAction>(
            *this, _selectedIndex, _selectedIndex - 1);
        action->redo();

        _baseController.undoStack().add_undo(std::move(action));
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
        auto action = std::make_unique<MoveUndoAction>(
            *this, _selectedIndex, _selectedIndex + 1);
        action->redo();

        _baseController.undoStack().add_undo(std::move(action));
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
ET& CappedVectorController<ET, LT, PT>::elementFromUndoRef(const UndoRef& ref)
{
    typename PT::element_type& p = PT::elementFromUndoRef(ref.parent);
    list_type& list = listFromParent<LT, typename PT::element_type>(p);

    return list.at(ref.index);
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
