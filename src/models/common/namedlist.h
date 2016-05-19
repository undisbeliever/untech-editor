#ifndef _UNTECH_MODELS_COMMON_NAMEDLIST_H_
#define _UNTECH_MODELS_COMMON_NAMEDLIST_H_

#include "namechecks.h"
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

namespace UnTech {

namespace Undo {
namespace Private {
template <class T>
class NamedListAddRemove;
}
}

/**
 * A basic map that enforces id names and child-parent references.
 *
 * Internally uses std::unique_ptr so that:
 *      * find is done on the pointer level.
 *      * undo engine can remove and insert in place.
 *
 * MEMORY: The owner (parent) class MUST exist when while the list exists.
 * THREADS: NOT THREAD SAFE
 */
template <class P, class T>
class NamedList {

    friend class UnTech::Undo::Private::NamedListAddRemove<T>;

public:
    template <typename IT>
    class dereference_iterator : public IT {
    public:
        dereference_iterator(IT it)
            : IT(it)
        {
        }
        std::pair<std::string, T&> operator*() const
        {
            auto& i = IT::operator*();
            return { i.first, *(i.second) };
        };
    };

    typedef dereference_iterator<typename std::map<std::string, std::unique_ptr<T>>::iterator> iterator;
    typedef dereference_iterator<typename std::map<std::string, std::unique_ptr<T>>::const_iterator> const_iterator;
    typedef dereference_iterator<typename std::map<std::string, std::unique_ptr<T>>::reverse_iterator> reverse_iterator;
    typedef dereference_iterator<typename std::map<std::string, std::unique_ptr<T>>::const_reverse_iterator> const_reverse_iterator;

public:
    NamedList() = delete;

    NamedList(P& owner)
        : _owner(owner)
        , _values()
        , _names()
    {
    }

    // returns a pointer as this may fail
    T* create(const std::string& name)
    {
        if (isNameValid(name) && !nameExists(name)) {
            auto newElem = std::make_unique<T>(_owner);
            T* newElemPtr = newElem.get();

            _values[name] = std::move(newElem);
            _names.insert({ newElemPtr, name });

            return newElemPtr;
        }

        return nullptr;
    }

    // returns a pointer as this may fail
    T* clone(T& e, const std::string& newName)
    {
        if (isNameValid(newName) && !nameExists(newName)) {
            auto newElem = std::make_unique<T>(e, _owner);
            T* newElemPtr = newElem.get();

            _values[newName] = std::move(newElem);
            _names.insert({ newElemPtr, newName });

            return newElemPtr;
        }

        return nullptr;
    }

    void remove(T* e)
    {
        auto it = _names.find(e);

        if (it != _names.end()) {
            _values.erase(it->second);
            _names.erase(it);
        }
    }

    bool changeName(T* e, const std::string& newName)
    {
        auto nIt = _names.find(e);

        if (nIt != _names.end()) {
            const std::string currentName = nIt->second;

            if (newName != currentName) {
                if (!isNameValid(newName) || nameExists(newName)) {
                    return false;
                }

                auto vIt = _values.find(currentName);
                std::unique_ptr<T> elem = std::move(vIt->second);

                _values.erase(vIt);
                _names.erase(nIt);

                _values[newName] = std::move(elem);
                _names.insert({ e, newName });
            }
            return true;
        }
        else {
            return false;
        }
    }
    bool changeName(T& e) { return changeName(&e); }

    bool nameExists(const std::string& name) const
    {
        auto it = _values.find(name);
        return it != _values.end();
    }

    std::pair<std::string, bool> getName(const T* e) const
    {
        const auto it = _names.find(e);
        if (it != _names.end()) {
            return { it->second, true };
        }

        return { std::string(), false };
    }
    std::pair<std::string, bool> getName(const T& e) const { return getName(&e); }

    T* getPtr(const std::string& name) const
    {
        auto it = _values.find(name);

        if (it != _values.end()) {
            return it->second.get();
        }
        else {
            return nullptr;
        }
    }

    // Expose the map
    T& at(const std::string& name) { return *_values.at(name).get(); }
    const T& at(const std::string& name) const { return *_values.at(name).get(); }

    inline size_t size() const { return _values.size(); }

    inline iterator begin() noexcept { return _values.begin(); }
    inline iterator end() noexcept { return _values.end(); }
    inline const_iterator begin() const noexcept { return _values.begin(); }
    inline const_iterator end() const noexcept { return _values.end(); }
    inline reverse_iterator rbegin() noexcept { return _values.rbegin(); }
    inline reverse_iterator rend() noexcept { return _values.rend(); }
    inline const_reverse_iterator rbegin() const noexcept { return _values.rbegin(); }
    inline const_reverse_iterator rend() const noexcept { return _values.rend(); }

protected:
    // Only allow these methods to be accessible by the undo module.
    // Prevent me from doing something stupid.

    std::unique_ptr<T> removeFrom(const std::string& name)
    {
        auto it = _values.find(name);

        if (it != _values.end()) {
            _names.erase(it->second.get());
            std::unique_ptr<T> ret = std::move(it->second);

            _values.erase(it);

            return std::move(ret);
        }
        else {
            return nullptr;
        }
    }

    void insertInto(std::unique_ptr<T> e, const std::string& name)
    {
        if (isNameValid(name) && !nameExists(name)) {
            _names.insert({ e.get(), name });
            _values[name] = std::move(e);
        }
    }

private:
    P& _owner;
    std::map<std::string, std::unique_ptr<T>> _values;
    std::unordered_map<const T*, std::string> _names;
};
}

#endif
