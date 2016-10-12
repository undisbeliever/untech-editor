#include "idmapcontroller.h"

namespace UnTech {
namespace Controller {

template <class T, class ParentT>
UnTech::idmap<T>& idmapFromParent(ParentT&);

template <typename ElementT, class ParentT>
const ElementT IdMapController<ElementT, ParentT>::BLANK_T = ElementT();

template <class T, class ParentT>
IdMapController<T, ParentT>::IdMapController(ParentT& parent)
    : _parent(parent)
    , _baseController(parent.baseController())
{
    parent.signal_selectedChanged().connect(sigc::mem_fun(
        *this, &IdMapController::reloadMap));

    this->_signal_mapChanged.connect(sigc::mem_fun(
        *this, &IdMapController::validateSelectedId));
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
        // ::TODO undo engine::

        _map->create(newId);
        onCreate(newId, _map->at(newId));

        _selectedId = newId;

        _signal_mapChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();
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
        // ::TODO undo engine::

        _map->clone(_selectedId, newId);
        _selectedId = newId;

        _signal_mapChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();
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
        // ::TODO undo engine::

        _map->rename(_selectedId, newId);
        _selectedId = newId;

        _signal_mapChanged.emit();
        _signal_selectedChanged.emit();
        _signal_anyChanged.emit();
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
        // ::TODO undo engine::

        _map->remove(_selectedId);

        selectNone();
        _signal_mapChanged.emit();
        _signal_anyChanged.emit();
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
T* IdMapController<T, PT>::elementFromUndoRef(const UndoRef& ref)
{
    auto* p = PT::elementFromUndoRef(ref.parent);
    if (p) {
        auto& map = idmapFromParent<T, typename PT::element_type>(*p);
        return &map.at(ref.id);
    }

    return nullptr;
}

template <class T, class PT>
void IdMapController<T, PT>::edit_selected(std::function<void(T&)> const& fun)
{
    class Action : public Undo::Action {
    public:
        Action() = delete;
        Action(IdMapController& controller, const T& value)
            : _controller(controller)
            , _ref(controller.undoRefForSelected())
            , _oldValue(value)
            , _newValue()
        {
        }
        virtual ~Action() override = default;

        virtual void undo() override
        {
            T* element = elementFromUndoRef(_ref);
            if (element) {
                *element = _oldValue;

                _controller._signal_dataChanged.emit();
                _controller._signal_anyChanged.emit();
            }
        }

        virtual void redo() override
        {
            T* element = elementFromUndoRef(_ref);
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

        void setNewValue(const T& value) { _newValue = value; }

    private:
        IdMapController& _controller;
        UndoRef _ref;
        const T _oldValue;
        T _newValue;
    };

    if (_map && _selectedId.isValid()) {
        auto& value = _map->at(_selectedId);

        auto action = std::make_unique<Action>(*this, value);

        fun(value);

        action->setNewValue(value);

        _baseController.undoStack().add_undo(std::move(action));
    }

    _signal_dataChanged.emit();
    _signal_anyChanged.emit();
}
}
}
