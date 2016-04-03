#ifndef _UNTECH_MODELS_COMMON_NAMEDLISTREF_H_
#define _UNTECH_MODELS_COMMON_NAMEDLISTREF_H_

#include "namechecks.h"
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>
#include <string>

namespace UnTech {

namespace Undo {
namespace Private {
template <class T>
class NamedListAddRemove;
}
}

/**
 * Operates the same as `NamedList` except it passes a reference
 * instead of a `std::shared_ptr` to the parent constructor.
 */
template <class P, class T>
class NamedListRef {

    friend class UnTech::Undo::Private::NamedListAddRemove<T>;

public:
    NamedListRef() = delete;

    NamedListRef(P& owner)
        : _owner(owner)
        , _values()
        , _names()
    {
    }

    std::shared_ptr<T> create(const std::string& name)
    {
        if (isNameValid(name) && !nameExists(name)) {
            auto e = std::make_shared<T>(_owner);

            _values.insert({ name, e });
            _names.insert({ e, name });

            return e;
        }

        return nullptr;
    }

    std::shared_ptr<T> clone(std::shared_ptr<T> e, const std::string& newName)
    {
        if (isNameValid(newName) && !nameExists(newName)) {
            auto newElem = e->clone(_owner);

            _values.insert({ newName, newElem });
            _names.insert({ newElem, newName });

            return newElem;
        }

        return nullptr;
    }

    void remove(std::shared_ptr<T> e)
    {
        auto it = _names.find(e);

        if (it != _names.end()) {
            _values.erase(it->second);
            _names.erase(it);
        }
    }

    bool changeName(std::shared_ptr<T> e, const std::string& newName)
    {
        auto it = _names.find(e);

        if (it != _names.end()) {
            const std::string currentName = it->second;

            if (newName != currentName) {
                if (!isNameValid(newName) || nameExists(newName)) {
                    return false;
                }

                _values.erase(currentName);
                _names.erase(it);

                _values.insert({ newName, e });
                _names.insert({ e, newName });
            }
            return true;
        }
        else {
            return false;
        }
    }

    bool nameExists(std::string name) const
    {
        auto it = _values.find(name);
        return it != _values.end();
    }

    std::pair<std::string, bool> getName(std::shared_ptr<T> e) const
    {
        auto it = _names.find(e);
        if (it != _names.end()) {
            return { it->second, true };
        }

        return { std::string(), false };
    }

    // Expose the map
    inline auto size() const { return _values.size(); }
    inline auto begin() const { return _values.begin(); }
    inline auto end() const { return _values.end(); }
    inline auto cbegin() const { return _values.cbegin(); }
    inline auto cend() const { return _values.cend(); }

protected:
    // Only allow these methods to be accessible by the undo module.
    // Prevent me from doing something stupid.

    void insertInto(std::shared_ptr<T> e, const std::string& name)
    {
        if (isNameValid(name) && !nameExists(name)) {
            _values.insert({ name, e });
            _names.insert({ e, name });
        }
    }

private:
    P& _owner;
    std::map<std::string, std::shared_ptr<T>> _values;
    std::unordered_map<std::shared_ptr<T>, std::string> _names;
};
}

#endif
