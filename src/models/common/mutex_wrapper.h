/*
 * This file is part of the UnTech Editor Suite.
 * Copyright (c) 2023, Marcus Rowe <undisbeliever@gmail.com>.
 * Distributed under The MIT License: https://opensource.org/licenses/MIT
 */

#pragma once

#include <concepts>
#include <gsl/pointers>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>

namespace UnTech {

template <typename T>
class mutex {
private:
    std::mutex _mutex;
    T _data;

public:
    mutex() = default;
    ~mutex() = default;

    // Forbid moving
    mutex(const mutex&) = delete;
    mutex(mutex&&) = delete;
    mutex& operator=(const mutex&) = delete;
    mutex& operator=(mutex&&) = delete;

    template <typename Function>
        requires std::invocable<Function, T&>
    void access(Function f)
    {
        std::lock_guard lock(_mutex);

        f(_data);
    }

    // I wanted to write a c++20 concept that would restrict the return
    // type of Function is `std::shared_ptr<const U>` or `std::integral`,
    // but I never figured it out.
    // Instead, we have separate access methods for each return type.

    // Returning a std::shared_ptr<const U> from T is safe
    template <typename U, typename Function>
        requires std::invocable<Function, T&>
    std::shared_ptr<const U> access_and_return_const_shared_ptr(Function f)
    {
        std::lock_guard lock(_mutex);

        return f(_data);
    }

    // Returning an unsigned from T is safe
    template <typename Function>
        requires std::invocable<Function, T&>
    unsigned access_and_return_unsigned(Function f)
    {
        std::lock_guard lock(_mutex);

        return f(_data);
    }
};

template <typename T>
class shared_mutex {
private:
    mutable std::shared_mutex _mutex;
    T _data;

public:
    shared_mutex() = default;
    ~shared_mutex() = default;

    shared_mutex(T d)
        : _mutex()
        , _data(std::move(d))
    {
    }

    // Forbid moving
    shared_mutex(const shared_mutex&) = delete;
    shared_mutex(shared_mutex&&) = delete;
    shared_mutex& operator=(const shared_mutex&) = delete;
    shared_mutex& operator=(shared_mutex&&) = delete;

    template <typename Function>
        requires std::invocable<Function, const T&>
    void read(Function f) const
    {
        std::shared_lock lock(_mutex);

        // Ensure `f` only has read-only access to `_data`
        const T& const_d = _data;
        f(const_d);
    }

    template <typename Function>
        requires std::invocable<Function, T&>
    void write(Function f)
    {
        std::lock_guard lock(_mutex);

        f(_data);
    }

    template <typename Function>
        requires std::invocable<Function, T&>
    void tryWrite(Function f)
    {
        std::unique_lock lock(_mutex, std::try_to_lock);

        if (lock.owns_lock()) {
            f(_data);
        }
    }

    // I wanted to write a c++20 concept that would restrict the return
    // type of Function is `std::shared_ptr<const U>` or `std::integral`,
    // but I never figured it out.
    // Instead, we have separate access methods for each return type.

    // Returning a bool is safe
    template <typename Function>
    bool write_and_return_bool(Function f)
        requires std::invocable<Function, T&>
    {
        std::lock_guard lock(_mutex);

        return f(_data);
    }

    // Returning an size_t from T is safe
    template <typename Function>
    size_t read_and_return_size_t(Function f) const
        requires std::invocable<Function, const T&>
    {
        std::lock_guard lock(_mutex);

        // Ensure `f` only has read-only access to `_data`
        const T& const_d = _data;
        return f(const_d);
    }

    // Returning a uint64_t from T is safe
    template <typename Function>
    uint64_t read_and_return_uint64_t(Function f) const
        requires std::invocable<Function, const T&>
    {
        std::lock_guard lock(_mutex);

        // Ensure `f` only has read-only access to `_data`
        const T& const_d = _data;
        return f(const_d);
    }

    // Returning an enum is safe
    template <typename U, typename Function>
    U read_and_return_enum(Function f) const
        requires std::invocable<Function, T&> and std::is_enum_v<U>
    {
        std::lock_guard lock(_mutex);

        // Ensure `f` only has read-only access to `_data`
        const T& const_d = _data;
        return f(const_d);
    }

    // Returning a std::shared_ptr<const U> from T is safe
    template <typename U, typename Function>
    std::shared_ptr<const U> read_and_return_const_shared_ptr(Function f) const
        requires std::invocable<Function, const T&>
    {
        std::lock_guard lock(_mutex);

        // Ensure `f` only has read-only access to `_data`
        const T& const_d = _data;
        return f(const_d);
    }

    // Returning a size_t and a std::shared_ptr<const U> from T is safe
    template <typename U, typename Function>
    std::optional<std::pair<size_t, gsl::not_null<std::shared_ptr<const U>>>> read_and_return_index_and_const_shared_ptr(Function f) const
        requires std::invocable<Function, const T&>
    {
        std::lock_guard lock(_mutex);

        // Ensure `f` only has read-only access to `_data`
        const T& const_d = _data;
        return f(const_d);
    }
};

template <typename T>
class shared_mutex<std::unique_ptr<T>> {
private:
    mutable std::shared_mutex _mutex;
    const std::unique_ptr<T> _data;

public:
    ~shared_mutex() = default;

    shared_mutex(std::unique_ptr<T>&& v)
        : _mutex{}
        , _data(std::move(v))
    {
    }

    // Forbid moving
    shared_mutex(const shared_mutex&) = delete;
    shared_mutex(shared_mutex&&) = delete;
    shared_mutex& operator=(const shared_mutex&) = delete;
    shared_mutex& operator=(shared_mutex&&) = delete;

    template <typename Function>
        requires std::invocable<Function, const T&>
    void read(Function f) const
    {
        std::shared_lock lock(_mutex);

        // Ensure `f` only has read-only access to `_data`
        const T& const_d = *_data;
        f(const_d);
    }

    template <typename Function>
        requires std::invocable<Function, T&>
    void write(Function f)
    {
        std::lock_guard lock(_mutex);

        f(*_data);
    }

    template <typename Function>
        requires std::invocable<Function, T&>
    void tryWrite(Function f)
    {
        std::unique_lock lock(_mutex, std::try_to_lock);

        if (lock.owns_lock()) {
            f(*_data);
        }
    }
};

}
