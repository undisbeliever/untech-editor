#ifndef _UNTECH_MODELS_COMMON_ORDEREDLIST_H_
#define _UNTECH_MODELS_COMMON_ORDEREDLIST_H_

#include <memory>
#include <string>
#include <vector>

namespace UnTech {

namespace Undo {
namespace Private {
template <class T>
class OrderedListAddRemove;
}
}

/**
 * A basic list interface that enforces child-parent references.
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

    friend class UnTech::Undo::Private::OrderedListAddRemove<T>;

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
    }

    T& create()
    {
        _list.emplace_back(std::make_unique<T>(_owner));
        return *(_list.back());
    }

    T& clone(const T& e)
    {
        _list.emplace_back(std::make_unique<T>(e, _owner));
        return *(_list.back());
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

    bool isLast(const T* e) const
    {
        return _list.size() > 0 && _list.back().get() == e;
    }

    bool contains(const T* e) const
    {
        auto it = findIt(e);
        return it != _list.end();
    }

    // Expose the list
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

    int indexOf(T* e)
    {
        auto it = findIt(e);

        if (it != _list.end()) {
            return std::distance(_list.begin(), it);
        }
        else {
            return -1;
        }
    }

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
        auto it = _list.begin() + index;
        _list.insert(it, std::move(e));
    }

private:
    P& _owner;
    std::vector<std::unique_ptr<T>> _list;
};
}

#endif
