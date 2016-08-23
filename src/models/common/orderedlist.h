#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace UnTech {

namespace Controller {
namespace Private {
template <class T>
class OrderedListAddRemove;
}
}

/**
 * A basic list interface that enforces child-parent references.
 *
 * Will throw an exception if list length exceeds maxSize.
 *
 * Internally uses std::unique_ptr so that:
 *      * std::find is done on the pointer level.
 *      * undo engine can remove and insert in place.
 *
 * MEMORY: The owner (parent) class MUST exist when while the list exists.
 * THREADS: NOT THREAD SAFE
 */
template <class P, class T>
class OrderedList {

    friend class UnTech::Controller::Private::OrderedListAddRemove<T>;

public:
    template <typename IT>
    class dereference_iterator : public IT {
    public:
        dereference_iterator(IT it)
            : IT(it)
        {
        }
        T& operator*() const { return *(IT::operator*()); };
    };

    typedef dereference_iterator<typename std::vector<std::unique_ptr<T>>::iterator> iterator;
    typedef dereference_iterator<typename std::vector<std::unique_ptr<T>>::const_iterator> const_iterator;
    typedef dereference_iterator<typename std::vector<std::unique_ptr<T>>::reverse_iterator> reverse_iterator;
    typedef dereference_iterator<typename std::vector<std::unique_ptr<T>>::const_reverse_iterator> const_reverse_iterator;

public:
    OrderedList() = default;

    OrderedList(P& owner)
        : _owner(owner)
        , _list()
    {
        _maxSize = _list.max_size();
    }

    OrderedList(P& owner, unsigned maxSize)
        : _owner(owner)
        , _maxSize(maxSize)
        , _list()
    {
        assert(_list.max_size() >= _maxSize);

        if (_maxSize <= 32) {
            _list.reserve(_maxSize);
        }
    }

    T& create()
    {
        if (_list.size() < _maxSize) {
            _list.emplace_back(std::make_unique<T>(_owner));
            return *(_list.back());
        }
        else {
            throw std::length_error("Too many items in ordered list");
        }
    }

    T& clone(const T& e)
    {
        if (_list.size() < _maxSize) {
            _list.emplace_back(std::make_unique<T>(e, _owner));
            return *(_list.back());
        }
        else {
            throw std::length_error("Too many items in ordered list");
        }
    }

    void remove(T* e)
    {
        auto it = findIt(e);

        if (it != _list.end()) {
            _list.erase(it);
        }
    }

    bool moveUp(T* e)
    {
        auto it = findIt(e);

        if (it != _list.end()) {
            if (it != _list.begin()) {
                auto other = it - 1;

                iter_swap(it, other);
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
                return true;
            }
        }
        return false;
    }

    bool isFirst(const T* e) const
    {
        return _list.size() > 0 && _list.front().get() == e;
    }

    T& first() { return *_list.at(0).get(); }
    const T& first() const { return *_list.at(0).get(); }

    bool isLast(const T* e) const
    {
        return _list.size() > 0 && _list.back().get() == e;
    }

    T& last() { return *_list.at(_list.size() - 1).get(); }
    const T& last() const { return *_list.at(_list.size() - 1).get(); }

    bool contains(const T* e) const
    {
        auto it = findIt(e);
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

    inline unsigned maxSize() const { return _maxSize; }
    inline bool canCreate() const { return _list.size() < _maxSize; }

    // Expose the list
    T& at(size_t i) { return *_list.at(i).get(); }
    const T& at(size_t i) const { return *_list.at(i).get(); }

    inline size_t size() const { return _list.size(); }

    inline iterator begin() noexcept { return _list.begin(); }
    inline iterator end() noexcept { return _list.end(); }
    inline const_iterator begin() const noexcept { return _list.begin(); }
    inline const_iterator end() const noexcept { return _list.end(); }
    inline reverse_iterator rbegin() noexcept { return _list.rbegin(); }
    inline reverse_iterator rend() noexcept { return _list.rend(); }
    inline const_reverse_iterator rbegin() const noexcept { return _list.rbegin(); }
    inline const_reverse_iterator rend() const noexcept { return _list.rend(); }

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
            std::unique_ptr<T> ret = std::move(*it);

            _list.erase(it);

            return std::move(ret);
        }
        else {
            return nullptr;
        }
    }

    void insertAtIndex(std::unique_ptr<T> e, size_t index)
    {
        if (_list.size() < _maxSize) {
            auto it = _list.begin() + index;
            _list.insert(it, std::move(e));
        }
        else {
            throw std::length_error("Too many items in ordered list");
        }
    }

private:
    P& _owner;
    unsigned _maxSize;
    std::vector<std::unique_ptr<T>> _list;
};
}
