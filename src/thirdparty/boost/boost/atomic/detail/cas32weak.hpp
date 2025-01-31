#ifndef BOOST_ATOMIC_DETAIL_CAS32WEAK_HPP
#define BOOST_ATOMIC_DETAIL_CAS32WEAK_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  Copyright (c) 2011 Helge Bahmann

#include <boost/memory_order.hpp>
#include <boost/atomic/detail/base.hpp>

namespace boost {
namespace atomics {
namespace detail {

/* integral types */

template<typename T, bool Sign>
class base_atomic<T, int, 1, Sign> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef T difference_type;
    typedef uint32_t storage_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before_store(order);
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
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);

        storage_type expected_s = (storage_type) expected;
        storage_type desired_s = (storage_type) desired;

        bool success = platform_cmpxchg32(expected_s, desired_s, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
            expected = (value_type) expected_s;
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original + v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original - v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original & v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original | v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original ^ v, order, memory_order_relaxed));
        return original;
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
class base_atomic<T, int, 2, Sign> {
    typedef base_atomic this_type;
    typedef T value_type;
    typedef T difference_type;
    typedef uint32_t storage_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before_store(order);
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
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);

        storage_type expected_s = (storage_type) expected;
        storage_type desired_s = (storage_type) desired;

        bool success = platform_cmpxchg32(expected_s, desired_s, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
            expected = (value_type) expected_s;
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original + v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original - v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original & v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original | v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original ^ v, order, memory_order_relaxed));
        return original;
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
        platform_fence_before_store(order);
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
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);

        bool success = platform_cmpxchg32(expected, desired, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
    }

    value_type
    fetch_add(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original + v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_sub(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original - v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_and(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original & v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_or(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original | v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_xor(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original ^ v, order, memory_order_relaxed));
        return original;
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
    typedef ptrdiff_t difference_type;
public:
    explicit base_atomic(value_type v) : v_(v) {}
    base_atomic(void) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        platform_fence_before_store(order);
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
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);

        bool success = platform_cmpxchg32(expected, desired, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
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
        platform_fence_before_store(order);
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
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        platform_fence_before(success_order);

        bool success = platform_cmpxchg32(expected, desired, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
    }

    value_type
    fetch_add(difference_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original + v, order, memory_order_relaxed));
        return original;
    }

    value_type
    fetch_sub(difference_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, original - v, order, memory_order_relaxed));
        return original;
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
    explicit base_atomic(value_type v) : v_(0)
    {
        memcpy(&v_, &v, sizeof(value_type));
    }
    base_atomic(void) : v_(0) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        storage_type tmp = 0;
        memcpy(&tmp, &v, sizeof(value_type));
        platform_fence_before_store(order);
        const_cast<volatile storage_type &>(v_) = tmp;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        storage_type tmp = const_cast<const volatile storage_type &>(v_);
        platform_fence_after_load(order);

        value_type v;
        memcpy(&v, &tmp, sizeof(value_type));
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        storage_type expected_s = 0, desired_s = 0;
        memcpy(&expected_s, &expected, sizeof(value_type));
        memcpy(&desired_s, &desired, sizeof(value_type));

        platform_fence_before(success_order);

        bool success = platform_cmpxchg32(expected_s, desired_s, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
            memcpy(&expected, &expected_s, sizeof(value_type));
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
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
    explicit base_atomic(value_type v) : v_(0)
    {
        memcpy(&v_, &v, sizeof(value_type));
    }
    base_atomic(void) : v_(0) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        storage_type tmp = 0;
        memcpy(&tmp, &v, sizeof(value_type));
        platform_fence_before_store(order);
        const_cast<volatile storage_type &>(v_) = tmp;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        storage_type tmp = const_cast<const volatile storage_type &>(v_);
        platform_fence_after_load(order);

        value_type v;
        memcpy(&v, &tmp, sizeof(value_type));
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        storage_type expected_s = 0, desired_s = 0;
        memcpy(&expected_s, &expected, sizeof(value_type));
        memcpy(&desired_s, &desired, sizeof(value_type));

        platform_fence_before(success_order);

        bool success = platform_cmpxchg32(expected_s, desired_s, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
            memcpy(&expected, &expected_s, sizeof(value_type));
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
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
    explicit base_atomic(value_type v) : v_(0)
    {
        memcpy(&v_, &v, sizeof(value_type));
    }
    base_atomic(void) : v_(0) {}

    void
    store(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        storage_type tmp = 0;
        memcpy(&tmp, &v, sizeof(value_type));
        platform_fence_before_store(order);
        const_cast<volatile storage_type &>(v_) = tmp;
        platform_fence_after_store(order);
    }

    value_type
    load(memory_order order = memory_order_seq_cst) const volatile
    {
        storage_type tmp = const_cast<const volatile storage_type &>(v_);
        platform_fence_after_load(order);

        value_type v;
        memcpy(&v, &tmp, sizeof(value_type));
        return v;
    }

    value_type
    exchange(value_type v, memory_order order = memory_order_seq_cst) volatile
    {
        value_type original = load(memory_order_relaxed);
        do {
        } while (!compare_exchange_weak(original, v, order, memory_order_relaxed));
        return original;
    }

    bool
    compare_exchange_weak(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        storage_type expected_s = 0, desired_s = 0;
        memcpy(&expected_s, &expected, sizeof(value_type));
        memcpy(&desired_s, &desired, sizeof(value_type));

        platform_fence_before(success_order);

        bool success = platform_cmpxchg32(expected_s, desired_s, &v_);

        if (success) {
            platform_fence_after(success_order);
        } else {
            platform_fence_after(failure_order);
            memcpy(&expected, &expected_s, sizeof(value_type));
        }

        return success;
    }

    bool
    compare_exchange_strong(
        value_type & expected,
        value_type desired,
        memory_order success_order,
        memory_order failure_order) volatile
    {
        for(;;) {
            value_type tmp = expected;
            if (compare_exchange_weak(tmp, desired, success_order, failure_order))
                return true;
            if (tmp != expected) {
                expected = tmp;
                return false;
            }
        }
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

}
}
}

#endif
