#ifndef BOOST_ATOMIC_DETAIL_GCC_SPARC_HPP
#define BOOST_ATOMIC_DETAIL_GCC_SPARC_HPP

//  Copyright (c) 2010 Helge Bahmann
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

namespace boost {
namespace atomics {
namespace detail {

static inline void
platform_fence_before(memory_order order)
{
    switch(order) {
        case memory_order_relaxed:
        case memory_order_acquire:
        case memory_order_consume:
            break;
        case memory_order_release:
        case memory_order_acq_rel:
            __asm__ __volatile__ ("membar #StoreStore | #LoadStore" ::: "memory");
            /* release */
            break;
        case memory_order_seq_cst:
            __asm__ __volatile__ ("membar #Sync" ::: "memory");
            /* seq */
            break;
    }
}

static inline void
platform_fence_after(memory_order order)
{
    switch(order) {
        case memory_order_relaxed:
        case memory_order_release:
            break;
        case memory_order_acquire:
        case memory_order_acq_rel:
            __asm__ __volatile__ ("membar #LoadLoad | #LoadStore" ::: "memory");
            /* acquire */
            break;
        case memory_order_consume:
            /* consume */
            break;
        case memory_order_seq_cst:
            __asm__ __volatile__ ("membar #Sync" ::: "memory");
            /* seq */
            break;
        default:;
    }
}

static inline void
platform_fence_after_store(memory_order order)
{
    switch(order) {
        case memory_order_seq_cst:
            __asm__ __volatile__ ("membar #Sync" ::: "memory");
        default:;
    }
}


static inline void
platform_fence_after_load(memory_order order)
{
    platform_fence_after(order);
}

}
}

class atomic_flag {
private:
    atomic_flag(const atomic_flag &) /* = delete */ ;
    atomic_flag & operator=(const atomic_flag &) /* = delete */ ;
    uint32_t v_;
public:
    atomic_flag(void) : v_(false) {}

    void
    clear(memory_order order = memory_order_seq_cst) volatile
    {
        atomics::detail::platform_fence_before(order);
        const_cast<volatile uint32_t &>(v_) = 0;
        atomics::detail::platform_fence_after_store(order);
    }

    bool
    test_and_set(memory_order order = memory_order_seq_cst) volatile
    {
        atomics::detail::platform_fence_before(order);
        uint32_t tmp = 1;
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (tmp)
            : "r" (&v_), "r" (0)
            : "memory"
        );
        atomics::detail::platform_fence_after(order);
        return tmp;
    }
};

} /* namespace boost */

#define BOOST_ATOMIC_FLAG_LOCK_FREE 2

#include <boost/atomic/detail/base.hpp>

#if !defined(BOOST_ATOMIC_FORCE_FALLBACK)

#define BOOST_ATOMIC_CHAR_LOCK_FREE 2
#define BOOST_ATOMIC_CHAR16_T_LOCK_FREE 2
#define BOOST_ATOMIC_CHAR32_T_LOCK_FREE 2
#define BOOST_ATOMIC_WCHAR_T_LOCK_FREE 2
#define BOOST_ATOMIC_SHORT_LOCK_FREE 2
#define BOOST_ATOMIC_INT_LOCK_FREE 2
#define BOOST_ATOMIC_LONG_LOCK_FREE 2
#define BOOST_ATOMIC_LLONG_LOCK_FREE 0
#define BOOST_ATOMIC_POINTER_LOCK_FREE 2
#define BOOST_ATOMIC_BOOL_LOCK_FREE 2

namespace boost {

#define BOOST_ATOMIC_THREAD_FENCE 2
static inline void
atomic_thread_fence(memory_order order)
{
    switch(order) {
        case memory_order_relaxed:
            break;
        case memory_order_release:
            __asm__ __volatile__ ("membar #StoreStore | #LoadStore" ::: "memory");
            break;
        case memory_order_acquire:
            __asm__ __volatile__ ("membar #LoadLoad | #LoadStore" ::: "memory");
            break;
        case memory_order_acq_rel:
            __asm__ __volatile__ ("membar #LoadLoad | #LoadStore | #StoreStore" ::: "memory");
            break;
        case memory_order_consume:
            break;
        case memory_order_seq_cst:
            __asm__ __volatile__ ("membar #Sync" ::: "memory");
            break;
        default:;
    }
}

#define BOOST_ATOMIC_SIGNAL_FENCE 2
static inline void
atomic_signal_fence(memory_order)
{
    __asm__ __volatile__ ("" ::: "memory");
}

namespace atomics {
namespace detail {

/* integral types */

template<typename T>
class base_atomic<T, int, 1, true> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef T difference_type;
    typedef int32_t storage_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) : v_(0) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before(order);
        const_cast<volatile storage_type &>(v_) = v;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        value_type v = const_cast<const volatile storage_type &>(v_);
        platform_fence_after_load(order);
        return v;
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp + v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp - v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);
        storage_type desired_s = desired;
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired_s)
            : "r" (&v_), "r" ((storage_type)expected)
            : "memory"
        );
        desired = desired_s;
        bool success = (desired == expected);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        expected = desired;
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp & v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp | v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp ^ v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_INTEGRAL_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    storage_type v_;
};

template<typename T>
class base_atomic<T, int, 1, false> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef T difference_type;
    typedef uint32_t storage_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) : v_(0) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before(order);
        const_cast<volatile storage_type &>(v_) = v;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        value_type v = const_cast<const volatile storage_type &>(v_);
        platform_fence_after_load(order);
        return v;
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp + v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp - v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);
        storage_type desired_s = desired;
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired_s)
            : "r" (&v_), "r" ((storage_type)expected)
            : "memory"
        );
        desired = desired_s;
        bool success = (desired == expected);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        expected = desired;
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp & v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp | v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp ^ v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_INTEGRAL_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    storage_type v_;
};

template<typename T>
class base_atomic<T, int, 2, true> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef T difference_type;
    typedef int32_t storage_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) : v_(0) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before(order);
        const_cast<volatile storage_type &>(v_) = v;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        value_type v = const_cast<const volatile storage_type &>(v_);
        platform_fence_after_load(order);
        return v;
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp + v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp - v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);
        storage_type desired_s = desired;
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired_s)
            : "r" (&v_), "r" ((storage_type)expected)
            : "memory"
        );
        desired = desired_s;
        bool success = (desired == expected);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        expected = desired;
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp & v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp | v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp ^ v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_INTEGRAL_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    storage_type v_;
};

template<typename T>
class base_atomic<T, int, 2, false> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef T difference_type;
    typedef uint32_t storage_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) : v_(0) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before(order);
        const_cast<volatile storage_type &>(v_) = v;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        value_type v = const_cast<const volatile storage_type &>(v_);
        platform_fence_after_load(order);
        return v;
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp + v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp - v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);
        storage_type desired_s = desired;
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired_s)
            : "r" (&v_), "r" ((storage_type)expected)
            : "memory"
        );
        desired = desired_s;
        bool success = (desired == expected);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        expected = desired;
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp & v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp | v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp ^ v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_INTEGRAL_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    storage_type v_;
};

template<typename T, bool Sign>
class base_atomic<T, int, 4, Sign> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef T difference_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before(order);
        const_cast<volatile value_type &>(v_) = v;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        value_type v = const_cast<const volatile value_type &>(v_);
        platform_fence_after_load(order);
        return v;
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp + v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp - v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired)
            : "r" (&v_), "r" (expected)
            : "memory"
        );
        bool success = (desired == expected);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        expected = desired;
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp & v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp | v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp ^ v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_INTEGRAL_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    value_type v_;
};

/* pointer types */

template<bool Sign>
class base_atomic<void *, void *, 4, Sign> {
    typedef base_atomic this_type;
    typedef void * value_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before(order);
        const_cast<volatile value_type &>(v_) = v;
        platform_fence_after_store(order);
    }

    value_type load(memory_order order = memory_order_seq_cst) const volatile
    {
        value_type v = const_cast<const volatile value_type &>(v_);
        platform_fence_after_load(order);
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired)
            : "r" (&v_), "r" (expected)
            : "memory"
        );
        bool success = (desired == expected);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        expected = desired;
        return success;
    }


    bool compare_exchange_weak(value_type & expected, value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_BASE_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    value_type v_;
};

template<typename T, bool Sign>
class base_atomic<T *, void *, 4, Sign> {
    typedef base_atomic this_type;
    typedef T * value_type;
    typedef ptrdiff_t difference_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before(order);
        const_cast<volatile value_type &>(v_) = v;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        value_type v = const_cast<const volatile value_type &>(v_);
        platform_fence_after_load(order);
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired)
            : "r" (&v_), "r" (expected)
            : "memory"
        );
        bool success = (desired == expected);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        expected = desired;
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    value_type
    fetch_add(difference_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp + v, order, memory_order_relaxed));
        return tmp;
    }

    value_type
    fetch_sub(difference_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, tmp - v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_POINTER_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    value_type v_;
};

/* generic types */

template<typename T, bool Sign>
class base_atomic<T, void, 1, Sign> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef uint32_t storage_type;
public:
    explicit base_atomic(value_type v)
    {
        v_ = 0;
        memcpy(&v_, &v, sizeof(value_type));
    }
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        storage_type tmp = 0;
        memcpy(&tmp, &v, sizeof(value_type));
        platform_fence_before(order);
        const_cast<volatile storage_type &>(v_) = tmp;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        storage_type tmp = const_cast<volatile storage_type &>(v_);
        platform_fence_after_load(order);
        value_type v;
        memcpy(&v, &tmp, sizeof(value_type));
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        storage_type expected_s = 0, desired_s = 0;
        memcpy(&expected_s, &expected, sizeof(value_type));
        memcpy(&desired_s, &desired, sizeof(value_type));
        platform_fence_before(success_order);
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired_s)
            : "r" (&v_), "r" (expected_s)
            : "memory"
        );
        bool success = (desired_s == expected_s);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        memcpy(&expected, &desired_s, sizeof(value_type));
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_BASE_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    storage_type v_;
};

template<typename T, bool Sign>
class base_atomic<T, void, 2, Sign> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef uint32_t storage_type;
public:
    explicit base_atomic(value_type v)
    {
        v_ = 0;
        memcpy(&v_, &v, sizeof(value_type));
    }
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        storage_type tmp = 0;
        memcpy(&tmp, &v, sizeof(value_type));
        platform_fence_before(order);
        const_cast<volatile storage_type &>(v_) = tmp;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        storage_type tmp = const_cast<volatile storage_type &>(v_);
        platform_fence_after_load(order);
        value_type v;
        memcpy(&v, &tmp, sizeof(value_type));
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        storage_type expected_s = 0, desired_s = 0;
        memcpy(&expected_s, &expected, sizeof(value_type));
        memcpy(&desired_s, &desired, sizeof(value_type));
        platform_fence_before(success_order);
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired_s)
            : "r" (&v_), "r" (expected_s)
            : "memory"
        );
        bool success = (desired_s == expected_s);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        memcpy(&expected, &desired_s, sizeof(value_type));
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_BASE_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    storage_type v_;
};

template<typename T, bool Sign>
class base_atomic<T, void, 4, Sign> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef uint32_t storage_type;
public:
    explicit base_atomic(value_type v)
    {
        memcpy(&v_, &v, sizeof(value_type));
    }
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        storage_type tmp;
        memcpy(&tmp, &v, sizeof(value_type));
        platform_fence_before(order);
        const_cast<volatile storage_type &>(v_) = tmp;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        storage_type tmp = const_cast<volatile storage_type &>(v_);
        platform_fence_after_load(order);
        value_type v;
        memcpy(&v, &tmp, sizeof(value_type));
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type tmp = load(memory_order_relaxed);
        do {} while(!compare_exchange_weak(tmp, v, order, memory_order_relaxed));
        return tmp;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        storage_type expected_s, desired_s;
        memcpy(&expected_s, &expected, sizeof(value_type));
        memcpy(&desired_s, &desired, sizeof(value_type));
        platform_fence_before(success_order);
        __asm__ (
            "cas [%1], %2, %0"
            : "+r" (desired_s)
            : "r" (&v_), "r" (expected_s)
            : "memory"
        );
        bool success = (desired_s == expected_s);
        if (success)
            platform_fence_after(success_order);
        else
            platform_fence_after(failure_order);
        memcpy(&expected, &desired_s, sizeof(value_type));
        return success;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        return compare_exchange_strong(expected, desired, success_order, failure_order);
    }

    bool
    is_lock_free(void) const volatile
    {
        return true;
    }

    BOOST_ATOMIC_DECLARE_BASE_OPERATORS
private:
    base_atomic(const base_atomic &) /* = delete */ ;
    void operator=(const base_atomic &) /* = delete */ ;
    storage_type v_;
};

#endif /* !defined(BOOST_ATOMIC_FORCE_FALLBACK) */

}
}
}

#endif
