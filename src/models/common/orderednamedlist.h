#pragma once

#include "namechecks.h"
#include "optional.h"
#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace UnTech {

/**
 * A basic ordered map interface that enforces child-parent references.
 *
 * Will throw an exception if list length exceeds maxSize.
 *
 * Internally uses std::unique_ptr so that:
 *      * find is done on the pointer level.
 *      * undo engine can remove and insert in place.
 *
 * MEMORY: The owner (parent) class MUST exist when while the list exists.
 * THREADS: NOT THREAD SAFE
 */
template <class P, class T>
class OrderedNamedList {

    typedef std::vector<std::pair<T*, std::string>> _nltype_t;

public:
    template <typename IT>
    class dereference_iterator : public IT {
    public:
        dereference_iterator(IT it)
            : IT(it)
        {
        }
        std::pair<const std::string&, T&> operator*() const
        {
            auto& ret = IT::operator*();
            return { ret.second, *ret.first };
        };
    };

    typedef dereference_iterator<typename _nltype_t::iterator> iterator;
    typedef dereference_iterator<typename _nltype_t::const_iterator> const_iterator;
    typedef dereference_iterator<typename _nltype_t::reverse_iterator> reverse_iterator;
    typedef dereference_iterator<typename _nltype_t::const_reverse_iterator> const_reverse_iterator;

public:
    OrderedNamedList() = default;

    OrderedNamedList(P& owner)
        : _owner(owner)
        , _list()
        , _nameList()
        , _nameMap()
    {
        _maxSize = std::min(_list.max_size(), _nameList.max_size(), _nameMap.max_size());
    }

    OrderedNamedList(P& owner, unsigned maxSize)
        : _owner(owner)
        , _maxSize(maxSize)
        , _list()
        , _nameList()
        , _nameMap()
    {
        assert(_maxSize <= _list.max_size());
        assert(_maxSize <= _nameList.max_size());
        assert(_maxSize <= _nameMap.max_size());
    }

    // returns a pointer as this may fail
    T* create(const std::string& name)
    {
        if (_list.size() >= _maxSize) {
            throw std::length_error("Too many items in ordered named list");
        }

        if (isNameValid(name) && !nameExists(name)) {
            auto newElem = std::make_unique<T>(_owner);
            T* newElemPtr = newElem.get();

            _nameMap.insert({ name, newElemPtr });
            _nameList.emplace_back(newElemPtr, name);
            _list.emplace_back(std::move(newElem));

            return newElemPtr;
        }

        return nullptr;
    }

    // returns a pointer as this may fail
    T* clone(T& e, const std::string& newName)
    {
        if (_list.size() >= _maxSize) {
            throw std::length_error("Too many items in ordered named list");
        }

        if (isNameValid(newName) && !nameExists(newName)) {
            auto newElem = std::make_unique<T>(e, _owner);
            T* newElemPtr = newElem.get();

            _nameMap.insert({ newName, newElemPtr });
            _nameList.emplace_back(newElemPtr, newName);
            _list.emplace_back(std::move(newElem));

            return newElemPtr;
        }

        return nullptr;
    }

    void remove(T* e)
    {
        auto it = findIt(e);

        if (it != _list.end()) {
            auto nit = _nameList.begin() + std::distance(_list.begin(), it);

            _nameMap.erase(nit->second);
            _nameList.erase(nit);
            _list.erase(it);
        }
    }

    bool moveUp(T* e)
    {
        auto it = findIt(e);

        if (it != _list.end()) {
            if (it != _list.begin()) {
                iter_swap(it, it - 1);

                auto nit = _nameList.begin() + std::distance(_list.begin(), it);
                iter_swap(nit, nit - 1);

                return true;
            }
        }
        return false;
    }

    bool moveDown(T* e)
    {
        auto it = findIt(e);

        if (it != _list.end()) {
            auto other = it + 1;

            if (other != _list.end()) {
                iter_swap(it, other);

                auto nit = _nameList.begin() + std::distance(_list.begin(), it);
                iter_swap(nit, nit + 1);

                return true;
            }
        }
        return false;
    }

    bool isFirst(const T* e) const
    {
        return _list.size() > 0 && _list.front().get() == e;
    }

    bool isLast(const T* e) const
    {
        return _list.size() > 0 && _list.back().get() == e;
    }

    bool contains(const T* e) const
    {
        auto it = fintIt(e);
        return it != _list.end();
    }

    int indexOf(const T* e) const
    {
        auto it = findIt(e);

        if (it != _list.end()) {
            return std::distance(_list.begin(), it);
        }
        else {
            return -1;
        }
    }
    int indexOf(const T& e) const { return indexOf(&e); }

    bool changeName(T* e, const std::string& newName)
    {
        if (isNameValid(newName) && nameExists(newName)) {
            auto nit = std::find_if(_nameList.begin(), _nameList.end(),
                                    [e](std::pair<T*, std::string>& v) { return v.first == e; });

            if (nit != _nameList.begin()) {
                _nameMap.erase(nit->second);
                _nameMap.insert({ newName, nit->first });
                nit->second = newName;

                return true;
            }
        }
        return false;
    }
    bool changeName(T& e) { return changeName(&e); }

    bool nameExists(const std::string& name) const
    {
        auto it = _nameMap.find(name);
        return it != _nameMap.end();
    }

    optional<std::string> getName(const T* e) const
    {
        const auto nit = std::find_if(_nameList.begin(), _nameList.end(),
                                      [e](std::pair<T*, std::string>& v) { return v.first == e; });

        if (nit != _nameList.end()) {
            return nit->second;
        }

        return optional<std::string>();
    }
    optional<std::string> getName(const T& e) const { return getName(&e); }

    inline unsigned maxSize() const { return _maxSize; }
    inline bool canCreate() const { return _list.size() < _maxSize; }

    inline bool canCreate(const std::string& name) const
    {
        return canCreate() && isNameValid(name) && !nameExists(name);
    }

    // Expose the list
    T& at(size_t i) { return *_list.at(i).get(); }
    const T& at(size_t i) const { return *_list.at(i).get(); }

    inline size_t size() const { return _list.size(); }

    inline iterator begin() noexcept { return _nameList.begin(); }
    inline iterator end() noexcept { return _nameList.end(); }
    inline const_iterator begin() const noexcept { return _nameList.begin(); }
    inline const_iterator end() const noexcept { return _nameList.end(); }
    inline reverse_iterator rbegin() noexcept { return _nameList.rbegin(); }
    inline reverse_iterator rend() noexcept { return _nameList.rend(); }
    inline const_reverse_iterator rbegin() const noexcept { return _nameList.rbegin(); }
    inline const_reverse_iterator rend() const noexcept { return _nameList.rend(); }

protected:
    inline auto findIt(T* e)
    {
        return std::find_if(_list.begin(), _list.end(),
                            [e](std::unique_ptr<T>& p) { return p.get() == e; });
    }

    inline auto findIt(const T* e) const
    {
        return std::find_if(_list.cbegin(), _list.cend(),
                            [e](const std::unique_ptr<T>& p) { return p.get() == e; });
    }

protected:
    // Only allow these methods to be accessible by the undo module.
    // Prevent me from doing something stupid.

    std::unique_ptr<T> removeFromList(size_t index)
    {
        if (index < _list.size()) {
            auto it = _list.begin() + index;
            auto nit = _nameList.begin() + index;

            std::unique_ptr<T> ret = std::move(*it);

            _nameMap.erase(nit->second);
            _nameList.erase(nit);
            _list.erase(it);

            return std::move(ret);
        }
        else {
            return nullptr;
        }
    }

    void insertAtIndex(std::unique_ptr<T> e, const std::string& name, size_t index)
    {
        if (_list.size() >= _maxSize) {
            throw std::length_error("Too many items in ordered named list");
        }

        auto it = _list.begin() + index;
        _list.insert(it, std::move(e));

        const T* ptr = _list[index].get();

        auto nit = _list.begin() + index;

        _nameList.insert(nit, { ptr, name });
        _nameMap.insert({ ptr, name });
    }

private:
    P& _owner;
    unsigned _maxSize;
    std::vector<std::unique_ptr<T>> _list;
    std::vector<std::pair<T*, std::string>> _nameList;

    std::unordered_map<std::string, const T*> _nameMap;
};
}
