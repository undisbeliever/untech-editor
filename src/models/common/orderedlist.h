#ifndef _UNTECH_MODELS_COMMON_ORDEREDLIST_H_
#define _UNTECH_MODELS_COMMON_ORDEREDLIST_H_

#include <memory>
#include <vector>
#include <string>

namespace UnTech {

template <class P, class T>
class OrderedList {

public:
    OrderedList() = default;

    OrderedList(P& owner)
        : _owner(owner)
        , _list()
    {
    }

    std::shared_ptr<T> create()
    {
        std::shared_ptr<P> owner = _owner.ptr();
        auto e = std::make_shared<T>(owner);
        _list.push_back(e);

        return e;
    }

    std::shared_ptr<T> clone(std::shared_ptr<T> e)
    {
        std::shared_ptr<P> owner = _owner.ptr();
        auto newElem = std::make_shared<T>(*e, owner);
        _list.push_back(newElem);

        return newElem;
    }

    void remove(std::shared_ptr<T> e)
    {
        auto it = std::find(_list.begin(), _list.end(), e);

        if (it != _list.end()) {
            _list.erase(it);
        }
    }

    void moveUp(std::shared_ptr<T> e)
    {
        auto it = std::find(_list.begin(), _list.end(), e);

        if (it != _list.end()) {
            if (it != _list.begin()) {
                auto other = it - 1;

                iter_swap(it, other);
            }
        }
    }

    void moveDown(std::shared_ptr<T> e)
    {
        auto it = std::find(_list.begin(), _list.end(), e);

        if (it != _list.end()) {
            auto other = it + 1;

            if (other != _list.end()) {
                iter_swap(it, other);
            }
        }
    }

    bool isFirst(std::shared_ptr<T> e)
    {
        return _list.size() > 0 && _list.front() == e;
    }

    bool isLast(std::shared_ptr<T> e)
    {
        return _list.size() > 0 && _list.back() == e;
    }

    // Expose the list
    inline auto size() const { return _list.size(); }
    inline auto begin() const { return _list.begin(); }
    inline auto end() const { return _list.end(); }
    inline auto cbegin() const { return _list.cbegin(); }
    inline auto cend() const { return _list.cend(); }

private:
    P& _owner;
    std::vector<std::shared_ptr<T>> _list;
};
}

#endif
