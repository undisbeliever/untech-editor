#include "idmapcontroller.h"

namespace UnTech {
namespace Controller {

template <class T, class ParentT>
UnTech::idmap<T>& idmapFromParent(ParentT&);

template <class T, class PT>
class IdMapController<T, PT>::MementoUndoAction : public Undo::Action {
public:
    MementoUndoAction() = delete;
    MementoUndoAction(IdMapController& controller, const T& value)
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
        // ::TODO undo message::
        static const std::string s = "edit_selected";
        return s;
    }

    void setNewValue(const T& value) { _newValue = value; }

private:
    IdMapController& _controller;
    const UndoRef _ref;
    const T _oldValue;
    T _newValue;
};

template <class T, class PT>
class IdMapController<T, PT>::CreateUndoAction : public Undo::Action {
public:
    CreateUndoAction() = delete;
    CreateUndoAction(IdMapController& controller, const idstring& id)
        : _controller(controller)
        , _ref(controller.parent().undoRefForSelected())
        , _id(id)
        , _element(nullptr)
    {
    }
    virtual ~CreateUndoAction() override = default;

    virtual void undo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        map_type& map = idmapFromParent<T, typename PT::element_type>(p);

        _element = map.extractFrom(_id);

        _controller.validateSelectedId();
        _controller._signal_mapChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual void redo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        map_type& map = idmapFromParent<T, typename PT::element_type>(p);

        map.insertInto(_id, std::move(_element));

        _controller._signal_mapChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual const std::string& message() const override
    {
        // ::TODO undo message::
        static const std::string s = "Create";
        return s;
    }

private:
    IdMapController& _controller;
    const typename PT::UndoRef _ref;
    const idstring _id;
    std::unique_ptr<T> _element;
};

template <class T, class PT>
class IdMapController<T, PT>::RemoveUndoAction : public Undo::Action {
public:
    RemoveUndoAction() = delete;
    RemoveUndoAction(IdMapController& controller, const idstring& id)
        : _controller(controller)
        , _ref(controller.parent().undoRefForSelected())
        , _id(id)
        , _element(nullptr)
    {
    }
    virtual ~RemoveUndoAction() override = default;

    virtual void undo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        map_type& map = idmapFromParent<T, typename PT::element_type>(p);

        map.insertInto(_id, std::move(_element));

        _controller._signal_mapChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual void redo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        map_type& map = idmapFromParent<T, typename PT::element_type>(p);

        _element = map.extractFrom(_id);

        _controller.validateSelectedId();
        _controller._signal_mapChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual const std::string& message() const override
    {
        // ::TODO undo message::
        static const std::string s = "Remove";
        return s;
    }

private:
    IdMapController& _controller;
    const typename PT::UndoRef _ref;
    const idstring _id;
    std::unique_ptr<T> _element;
};

template <class T, class PT>
class IdMapController<T, PT>::RenameUndoAction : public Undo::Action {
public:
    RenameUndoAction() = delete;
    RenameUndoAction(IdMapController& controller,
                     const idstring& oldId, const idstring& newId)
        : _controller(controller)
        , _ref(controller.parent().undoRefForSelected())
        , _oldId(oldId)
        , _newId(newId)
    {
    }
    virtual ~RenameUndoAction() override = default;

    virtual void undo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        map_type& map = idmapFromParent<T, typename PT::element_type>(p);

        map.rename(_newId, _oldId);

        if (_controller._selectedId == _newId) {
            _controller._selectedId = _oldId;
        }

        _controller._signal_mapChanged.emit();
        _controller._signal_selectedChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual void redo() override
    {
        typename PT::element_type& p = PT::elementFromUndoRef(_ref);
        map_type& map = idmapFromParent<T, typename PT::element_type>(p);

        map.rename(_oldId, _newId);

        if (_controller._selectedId == _oldId) {
            _controller._selectedId = _newId;
        }

        _controller._signal_mapChanged.emit();
        _controller._signal_selectedChanged.emit();
        _controller._signal_anyChanged.emit();
    }

    virtual const std::string& message() const override
    {
        // ::TODO undo message::
        static const std::string s = "Rename";
        return s;
    }

private:
    IdMapController& _controller;
    const typename PT::UndoRef _ref;
    const idstring _oldId;
    const idstring _newId;
};

template <typename ElementT, class ParentT>
const ElementT IdMapController<ElementT, ParentT>::BLANK_T = ElementT();

template <class T, class ParentT>
IdMapController<T, ParentT>::IdMapController(ParentT& parent)
    : _parent(parent)
    , _baseController(parent.baseController())
{
    parent.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &IdMapController::reloadMap));
}

template <class T, class PT>
void IdMapController<T, PT>::reloadMap()
{
    typename PT::element_type* p = _parent.editable_selected();

    map_type* m = nullptr;
    if (p) {
        m = &idmapFromParent<T, typename PT::element_type>(*p);
    }
    if (m != _map || _map == nullptr) {
        _selectedId = idstring();
        _map = m;

        _signal_selectedChanged.emit();
        _signal_mapChanged.emit();
        _signal_anyChanged.emit();
    }
    else {
        validateSelectedId();
    }
}

template <class T, class PT>
void IdMapController<T, PT>::selectNone()
{
    if (hasSelected()) {
        _selectedId = idstring();

        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();
    }
}

template <class T, class PT>
void IdMapController<T, PT>::selectId(const idstring& id)
{
    if (_map && _map->contains(id)) {
        if (id != _selectedId) {
            _selectedId = id;

            _signal_selectedChanged.emit();
            _signal_anyChanged.emit();
        }
    }
    else {
        selectNone();
    }
}

template <class T, class PT>
void IdMapController<T, PT>::validateSelectedId()
{
    if (_selectedId.isValid()) {
        if (_map == nullptr || !_map->contains(_selectedId)) {
            selectNone();
        }
    }
}

template <class T, class PT>
bool IdMapController<T, PT>::canCreate() const
{
    return _map;
}

template <class T, class PT>
bool IdMapController<T, PT>::canCreate(const idstring& newId) const
{
    return _map && newId.isValid() && !_map->contains(newId);
}

template <class T, class PT>
void IdMapController<T, PT>::create(const idstring& newId)
{
    if (canCreate(newId)) {
        _map->create(newId);
        onCreate(newId, _map->at(newId));

        _selectedId = newId;

        _signal_mapChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();

        _baseController.undoStack().add_undo(
            std::make_unique<CreateUndoAction>(*this, newId));
    }
}

template <class T, class PT>
void IdMapController<T, PT>::onCreate(const idstring&, T&)
{
}

template <class T, class PT>
bool IdMapController<T, PT>::canCloneSelected() const
{
    return _map && hasSelected();
}

template <class T, class PT>
bool IdMapController<T, PT>::canCloneSelected(const idstring& newId) const
{
    return _map && hasSelected() && newId.isValid() && !_map->contains(newId);
}

template <class T, class PT>
void IdMapController<T, PT>::cloneSelected(const idstring& newId)
{
    if (canCloneSelected(newId)) {
        _map->clone(_selectedId, newId);
        _selectedId = newId;

        _signal_mapChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();

        _baseController.undoStack().add_undo(
            std::make_unique<CreateUndoAction>(*this, newId));
    }
}

template <class T, class PT>
bool IdMapController<T, PT>::canRenameSelected() const
{
    return _map && hasSelected();
}

template <class T, class PT>
bool IdMapController<T, PT>::canRenameSelected(const idstring& newId) const
{
    return _map && hasSelected() && !_map->contains(newId);
}

template <class T, class PT>
void IdMapController<T, PT>::renameSelected(const idstring& newId)
{
    if (canRenameSelected(newId)) {
        auto action = std::make_unique<RenameUndoAction>(*this, _selectedId, newId);
        action->redo();

        _baseController.undoStack().add_undo(std::move(action));
    }
}

template <class T, class PT>
bool IdMapController<T, PT>::canRemoveSelected() const
{
    return _map && hasSelected();
}

template <class T, class PT>
void IdMapController<T, PT>::removeSelected()
{
    if (canRemoveSelected()) {
        auto action = std::make_unique<RemoveUndoAction>(*this, _selectedId);
        action->redo();

        _baseController.undoStack().add_undo(std::move(action));
    }
}

template <class T, class PT>
T* IdMapController<T, PT>::editable_selected()
{
    if (_map && hasSelected()) {
        return &_map->at(_selectedId);
    }
    else {
        return nullptr;
    }
}

template <class T, class PT>
typename IdMapController<T, PT>::UndoRef
IdMapController<T, PT>::undoRefForSelected() const
{
    if (_map && hasSelected()) {
        return {
            _parent.undoRefForSelected(),
            _selectedId
        };
    }
    else {
        throw std::logic_error("No element selected");
    }
}

template <class T, class PT>
T& IdMapController<T, PT>::elementFromUndoRef(const UndoRef& ref)
{
    typename PT::element_type& p = PT::elementFromUndoRef(ref.parent);
    map_type& map = idmapFromParent<T, typename PT::element_type>(p);

    return map.at(ref.id);
}

template <class T, class PT>
template <class UndoActionT>
void IdMapController<T, PT>::edit_selected(
    std::function<bool(const T&)> const& validate,
    std::function<void(T&)> const& fun)
{
    if (_map && _selectedId.isValid()) {
        auto& value = _map->at(_selectedId);

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
