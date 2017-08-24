// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

#ifndef _VOLATILE_H_
#define _VOLATILE_H_

#include <atomic>

// Defines the Volatile<T> type, designed to interoperate with the
// Volatile<T> VM type, but with a C++11 implementation.
//
// The original Volatile<T> was designed to address an inconsisteny
// between MSVC and GCC's implementations of the `volatile` storage class
// MSVC's `volatile` semantics are strictly stronger than what the
// standard requires - writes to volatile glvalues have release semantics
// and reads from volatile glvalues have acquire semantics. Relying on
// this non-standard behavior results in problematic threading issues
// when compiled by clang or GCC.
//
// Luckily, C++11 helps address this problem by providing the atomic
// header, which standardizes the semantics of atomic reads and writes.
// This header provides an implementation of the Volatile class using
// these C++ primitives.
template <typename T>
class Volatile
{
private:
    // The "volatile" data that is being shared across threads.
    std::atomic<T> m_data;

public:
    // Default constructor. Default-initializes the contents.
    Volatile() : m_data() {}

    // Initialization of a Volatile<T> from a const T&.
    Volatile(const T& value) : m_data(value) {}

    // Copy constructor.
    Volatile(const Volatile<T>& other) : m_data()
    {
        m_data = other.Load();
    }

    // Loads T with acquire semantics.
    T Load() const
    {
        return m_data.load(std::memory_order_acquire);
    }

    // Loads T with relaxed semantics.
    T LoadWithoutBarrier() const
    {
        return m_data.load(std::memory_order_relaxed);
    }

    // Stores to T with release semantics.
    void Store(const T& value)
    {
        m_data.store(value, std::memory_order_release);
    }

    // Stores to T with relaxed semantics.
    void StoreWithoutBarrier(const T& value)
    {
        m_data.store(value, std::memory_order_relaxed);
    }

    // Converts a Volatile<T> to T by performing an acquire load.
    operator T() const
    {
        return Load();
    }

    // Stores to a Volatile<T> by performing a release store.
    Volatile<T>& operator=(T val)
    {
        Store(val);
        return *this;
    }

    // Operator overloads for convenience
    T operator++()
    {
        return m_data.fetch_add(std::memory_order_acq_rel);
    }

    T operator--()
    {
        return m_data.fetch_sub(std::memory_order_acq_rel);
    }
};

#endif // _VOLATILE_H_
