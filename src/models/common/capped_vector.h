#pragma once

#include "humantypename.h"
#include <stdexcept>
#include <vector>

namespace UnTech {

/*
 * Generates an exception when vector size exceeds MAX_T.
 * As this exception's error message is passed to the user
 * the exception message exists.
 */
template <typename T, size_t MAX_T>
class capped_vector : public std::vector<T> {
    using parent = std::vector<T>;

    inline void _validate_insert(const size_t count = 1)
    {
        if (this->size() + count > MAX_T) {
            throw new std::runtime_error("Could not insert " + HumanTypeName<T>::value + ": (too many elements)");
        }
    }
    inline void _validate_count(const size_t count)
    {
        if (count > MAX_T) {
            throw new std::runtime_error("Too many elements in" + HumanTypeName<T>::value + " capped_vector");
        }
    }

public:
    const static size_t MAX_SIZE = MAX_T;
    const static std::string HUMAN_TYPE_NAME;

    typedef typename parent::iterator iterator;
    typedef typename parent::const_iterator const_iterator;
    typedef typename parent::size_type size_type;

    ~capped_vector() = default;
    capped_vector(const capped_vector&) = default;
    capped_vector(capped_vector&&) = default;
    capped_vector& operator=(const capped_vector& other) = default;
    capped_vector& operator=(capped_vector&& other) = default;

    capped_vector() = default;

    capped_vector& operator=(std::initializer_list<T> ilist)
    {
        _validate_count(ilist.size());
        parent::operator=(ilist);
        return *this;
    }

    void resize(size_type count)
    {
        _validate_count(count);
        parent::resize(count);
    }
    void resize(size_type count, const T& value)
    {
        _validate_count(count);
        parent::resize(count, value);
    }

    void assign(size_type count, const T& value)
    {
        _validate_count(count);
        parent::assign(count, value);
    }
    template <class InputIt>
    void assign(InputIt first, InputIt last)
    {
        _validate_count(std::distance(first, last));
        parent::assign(first, last);
    }
    void assign(std::initializer_list<T> ilist)
    {
        _validate_count(ilist.size());
        parent::assign(ilist);
    }

    iterator insert(const_iterator pos, const T& value)
    {
        _validate_insert();
        return parent::insert(pos, value);
    }
    iterator insert(const_iterator pos, T&& value)
    {
        _validate_insert();
        return parent::insert(pos, value);
    }
    iterator insert(const_iterator pos, size_type count, const T& value)
    {
        _validate_insert(count);
        return parent::insert(pos, count, value);
    }
    template <class InputIt>
    iterator insert(const_iterator pos, InputIt first, InputIt last)
    {
        _validate_insert(std::distance(first, last));
        return parent::insert(pos, first, last);
    }
    iterator insert(const_iterator pos, std::initializer_list<T> ilist)
    {
        _validate_insert(ilist.size());
        return parent::insert(pos, ilist);
    }

    template <class... Args>
    iterator emplace(const_iterator pos, Args&&... args)
    {
        _validate_insert();
        return parent::emplace(pos, args...);
    }

    void push_back(const T& value)
    {
        _validate_insert();
        parent::push_back(value);
    }
    void push_back(T&& value)
    {
        _validate_insert();
        parent::emplace_back(value);
    }

    template <class... Args>
    void emplace_back(Args&&... args)
    {
        _validate_insert();
        parent::emplace_back(args...);
    }

    bool can_insert(size_t count = 1)
    {
        return this->size() + count <= MAX_SIZE;
    }
};
}
