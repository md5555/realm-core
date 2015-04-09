/*************************************************************************
 *
 * REALM CONFIDENTIAL
 * __________________
 *
 *  [2011] - [2015] Realm Inc
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of Realm Incorporated and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to Realm Incorporated
 * and its suppliers and may be covered by U.S. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Realm Incorporated.
 *
 **************************************************************************/

#pragma once
#ifndef REALM_UTIL_OPTIONAL_HPP
#define REALM_UTIL_OPTIONAL_HPP

#include <realm/util/features.h>

#include <stdexcept> // std::logic_error
#include <functional> // std::less

namespace realm {
namespace util {

// Note: Should conform with the future std::nullopt_t and std::in_place_t.
static REALM_CONSTEXPR struct None { REALM_CONSTEXPR None(int) {} } none { 0 };
static REALM_CONSTEXPR struct InPlace { REALM_CONSTEXPR InPlace() {} } in_place;

// Note: Should conform with the future std::bad_optional_access.
struct BadOptionalAccess : std::logic_error {
    explicit BadOptionalAccess(const std::string& what_arg) : std::logic_error(what_arg) {}
    explicit BadOptionalAccess(const char* what_arg) : std::logic_error(what_arg) {}
};

// Note: Should conform with the future std::optional.
template <class T>
class Optional {
public:
    using value_type = T;

    Optional();
    Optional(None);
    Optional(Optional<T>&& other);
    Optional(const Optional<T>& other);

    Optional(T&& value);
    Optional(const T& value);

    template <class... Args>
    Optional(InPlace tag, Args&&...);
    // FIXME: std::optional specifies an std::initializer_list constructor overload as well.

    ~Optional();

    Optional<T>& operator=(None);
    Optional<T>& operator=(Optional<T>&& other);
    Optional<T>& operator=(const Optional<T>& other);
    template <class U>
    Optional<T>& operator=(U&& value);

    explicit operator bool() const;
    const T& value() const; // Throws
    T& value(); // Throws
    const T& operator*() const; // Throws
    T& operator*(); // Throws
    const T* operator->() const; // Throws
    T* operator->(); // Throws

    template <class U>
    T value_or(U&& value) const&;

    template <class U>
    T value_or(U&& value) &&;

    void swap(Optional<T>& other); // FIXME: Add noexcept() clause

    template <class... Args>
    void emplace(Args&&...);
    // FIXME: std::optional specifies an std::initializer_list overload for `emplace` as well.
private:
    // The boolean that indicates whether this Optional contains something is stored
    // as an extra byte at the end of the memory block holding the actual value.
    using Storage = typename std::aligned_storage<sizeof(T) + 1, alignof(T)>::type;
    Storage m_storage;

    T* ptr() { return reinterpret_cast<T*>(&m_storage); }
    uint8_t* engaged() { return reinterpret_cast<uint8_t*>(ptr() + 1); }
    const T* ptr() const { return reinterpret_cast<const T*>(&m_storage); }
    const uint8_t* engaged() const { return reinterpret_cast<const uint8_t*>(ptr() + 1); }

    bool is_engaged() const { return *engaged() != 0; }
    void set_engaged(bool b) { *engaged() = b; }
    void clear();
};

/// An Optional<void> is functionally equivalent to a bool.
template <>
class Optional<void> {
public:
    Optional() {}
    Optional(None) {}
    Optional(Optional<void>&&) = default;
    Optional(const Optional<void>&) = default;
    explicit operator bool() const { return m_engaged; }

    static Optional<void> some()
    {
        Optional<void> opt;
        opt.m_engaged = true;
        return opt;
    }
private:
    bool m_engaged = false;
};

/// An Optional<T&> is functionally equivalent to a pointer (but safer)
template <class T>
class Optional<T&> {
public:
    using value_type = T&;
    using target_type = typename std::decay<T>::type;

    Optional() {}
    Optional(None) : Optional() {}
    Optional(Optional<T&>&& other) = default;
    Optional(const Optional<T&>& other) = default;
    template <class U>
    Optional(std::reference_wrapper<U> ref) : m_ptr(&ref.get()) {}

    Optional(T& value) : m_ptr(&value) {}

    Optional<T&>& operator=(None) { m_ptr = nullptr; return *this; }
    Optional<T&>& operator=(Optional<T&>&& other) { std::swap(m_ptr, other.m_ptr); return *this; }
    Optional<T&>& operator=(const Optional<T&>& other) { m_ptr = other.m_ptr; return *this; }

    template <class U>
    Optional<T&>& operator=(std::reference_wrapper<U> ref) { m_ptr = &ref.get(); return *this; }

    explicit operator bool() const { return m_ptr; }
    const target_type& value() const; // Throws
    target_type& value(); // Throws
    const target_type& operator*() const { return value(); }
    target_type& operator*() { return value(); }
    const target_type* operator->() const { return &value(); }
    target_type* operator->() { return &value(); }

    void swap(Optional<T&> other); // FIXME: Add noexcept() clause
private:
    T* m_ptr = nullptr;
};

template <class T>
Optional<T>::Optional()
{
    set_engaged(false);
}

template <class T>
Optional<T>::Optional(None)
{
    set_engaged(false);
}

template <class T>
Optional<T>::Optional(Optional<T>&& other)
{
    set_engaged(other.is_engaged());
    if (is_engaged()) {
        new(ptr()) T(std::move(*other.ptr()));
    }
}

template <class T>
Optional<T>::Optional(const Optional<T>& other)
{
    set_engaged(other.is_engaged());
    if (is_engaged()) {
        new(ptr()) T(*other.ptr());
    }
}

template <class T>
Optional<T>::Optional(T&& value)
{
    set_engaged(true);
    new(ptr()) T(std::move(value));
}

template <class T>
Optional<T>::Optional(const T& value)
{
    set_engaged(true);
    new(ptr()) T(value);
}

template <class T>
template <class... Args>
Optional<T>::Optional(InPlace, Args&&... args)
{
    set_engaged(true);
    new(ptr()) T(std::forward<Args>(args)...);
}

template <class T>
Optional<T>::~Optional()
{
    if (is_engaged()) {
        ptr()->~T();
    }
}

template <class T>
void Optional<T>::clear()
{
    if (is_engaged()) {
        ptr()->~T();
        set_engaged(false);
    }
}

template <class T>
Optional<T>& Optional<T>::operator=(None)
{
    clear();
    return *this;
}

template <class T>
Optional<T>& Optional<T>::operator=(Optional<T>&& other)
{
    if (is_engaged()) {
        if (other.is_engaged()) {
            *ptr() = std::move(*other.ptr());
        }
        else {
            clear();
        }
    }
    else {
        if (other.is_engaged()) {
            new(ptr()) T(std::move(*other.ptr()));
        }
    }
    return *this;
}

template <class T>
Optional<T>& Optional<T>::operator=(const Optional<T>& other)
{
    if (is_engaged()) {
        if (other.is_engaged()) {
            *ptr() = *other.ptr();
        }
        else {
            clear();
        }
    }
    else {
        if (other.is_engaged()) {
            new(ptr()) T(*other.ptr());
        }
    }
    return *this;
}

template <class T>
template <class U>
Optional<T>& Optional<T>::operator=(U&& value)
{
    if (is_engaged()) {
        *ptr() = std::forward<U>(value);
    }
    else {
        new(ptr()) T(std::forward<U>(value));
        set_engaged(true);
    }
    return *this;
}

template <class T>
Optional<T>::operator bool() const
{
    return is_engaged();
}

template <class T>
const T& Optional<T>::value() const
{
    if (!is_engaged()) {
        throw BadOptionalAccess{"bad optional access"};
    }
    return *ptr();
}

template <class T>
T& Optional<T>::value()
{
    if (!is_engaged()) {
        throw BadOptionalAccess{"bad optional access"};
    }
    return *ptr();
}

template <class T>
const typename Optional<T&>::target_type& Optional<T&>::value() const
{
    if (!m_ptr) {
        throw BadOptionalAccess{"bad optional access"};
    }
    return *m_ptr;
}

template <class T>
typename Optional<T&>::target_type& Optional<T&>::value()
{
    if (!m_ptr) {
        throw BadOptionalAccess{"bad optional access"};
    }
    return *m_ptr;
}

template <class T>
const T& Optional<T>::operator*() const
{
    // Note: This differs from std::optional, which doesn't throw.
    return value();
}

template <class T>
T& Optional<T>::operator*()
{
    // Note: This differs from std::optional, which doesn't throw.
    return value();
}

template <class T>
const T* Optional<T>::operator->() const
{
    // Note: This differs from std::optional, which doesn't throw.
    return &value();
}

template <class T>
T* Optional<T>::operator->()
{
    // Note: This differs from std::optional, which doesn't throw.
    return &value();
}

template <class T>
template <class U>
T Optional<T>::value_or(U&& otherwise) const&
{
    if (is_engaged()) {
        return T(*ptr());
    }
    else {
        return T(std::forward<U>(otherwise));
    }
}

template <class T>
template <class U>
T Optional<T>::value_or(U&& otherwise) &&
{
    if (is_engaged()) {
        return T(std::move(*ptr()));
    }
    else {
        return T(std::forward<U>(otherwise));
    }
}

template <class T>
void Optional<T>::swap(Optional<T>& other)
{
    // FIXME: This might be optimizable.
    Optional<T> tmp = std::move(other);
    other = std::move(*this);
    *this = std::move(tmp);
}

template <class T>
template <class... Args>
void Optional<T>::emplace(Args&&... args)
{
    clear();
    new(ptr()) T(std::forward<Args>(args)...);
    set_engaged(true);
}


template <class T>
REALM_CONSTEXPR Optional<typename std::decay<T>::type>
make_optional(T&& value)
{
    using Type = typename std::decay<T>::type;
    return Optional<Type>(std::forward<T>(value));
}

template <class R>
struct FMapResult {
    template <class F>
    static auto get(F&& func) -> Optional<decltype(func())>
    {
        return func();
    }
};
template <>
struct FMapResult<void> {
    template <class F>
    static auto get(F&& func) -> Optional<void>
    {
        func();
        return Optional<void>::some();
    }
};

template <class T, class F>
auto fmap(Optional<T>& opt, F&& func) -> Optional<decltype(func(std::declval<T>()))>
{
    using R = decltype(func(std::declval<T>()));
    if (opt) {
        return FMapResult<R>::get([&](){ return func(*opt); });
    }
    return none;
}

template <class T, class F>
auto fmap(Optional<T>&& opt, F&& func) -> Optional<decltype(func(std::declval<T>()))>
{
    using R = decltype(func(std::declval<T>()));
    if (opt) {
        return FMapResult<R>::get([&](){ return func(std::move(*opt)); });
    }
    return none;
}

template <class T, class F>
auto fmap(const Optional<T>& opt, F&& func) -> Optional<decltype(func(std::declval<T>()))>
{
    using R = decltype(func(std::declval<T>()));
    if (opt) {
        return FMapResult<R>::get([&](){ return func(*opt); });
    }
    return none;
}

template <class T>
bool operator==(const Optional<T>& lhs, const Optional<T>& rhs)
{
    if (!lhs && !rhs) { return true; }
    if (lhs && rhs) { return *lhs == *rhs; }
    return false;
}

template <class T>
bool operator<(const Optional<T>& lhs, const Optional<T>& rhs)
{
    if (!rhs) { return false; }
    if (!lhs) { return true; }
    return std::less<T>{}(*lhs, *rhs);
}

template <class T>
bool operator==(const Optional<T>& lhs, None)
{
    return !bool(lhs);
}

template <class T>
bool operator<(const Optional<T>& lhs, None)
{
    static_cast<void>(lhs);
    return false;
}

template <class T>
bool operator==(None, const Optional<T>& rhs)
{
    return !bool(rhs);
}

template <class T>
bool operator<(None, const Optional<T>& rhs)
{
    return bool(rhs);
}

template <class T>
bool operator==(const Optional<T>& lhs, const T& rhs)
{
    return lhs ? *lhs == rhs : false;
}

template <class T>
bool operator<(const Optional<T>& lhs, const T& rhs)
{
    return lhs ? std::less<T>{}(*lhs, rhs) : true;
}

template <class T>
bool operator==(const T& lhs, const Optional<T>& rhs)
{
    return rhs ? lhs == *rhs : false;
}

template <class T>
bool operator<(const T& lhs, const Optional<T>& rhs)
{
    return rhs ? std::less<T>{}(lhs, *rhs) : false;
}

} // namespace util

using util::none;

} // namespace realm

#endif // REALM_UTIL_OPTIONAL_HPP
