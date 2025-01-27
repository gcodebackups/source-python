/*
 * Copyright (c) 2012 Glen Joseph Fernandes 
 * glenfe at live dot com
 *
 * Distributed under the Boost Software License, 
 * Version 1.0. (See accompanying file LICENSE_1_0.txt 
 * or copy at http://boost.org/LICENSE_1_0.txt)
 */
#ifndef BOOST_SMART_PTR_DETAIL_MAKE_ARRAY_HELPER_HPP
#define BOOST_SMART_PTR_DETAIL_MAKE_ARRAY_HELPER_HPP

#include <boost/type_traits/alignment_of.hpp>

namespace boost {
    namespace detail {
        template<typename T, typename Y = T>
        class make_array_helper {
            template<typename T2, typename Y2>
            friend class make_array_helper;
        public:
            typedef Y           value_type;
            typedef Y*          pointer;
            typedef const Y*    const_pointer;
            typedef Y&          reference;
            typedef const Y&    const_reference;
            typedef std::size_t size_type;
            typedef ptrdiff_t   difference_type;
            template<typename U>
            struct rebind {
                typedef make_array_helper<T, U> other;
            };
            make_array_helper(std::size_t size, T** data)
                : size(sizeof(T) * size),
                  data(data) {
            }
            make_array_helper(const make_array_helper& other)
                : size(other.size),
                  data(other.data) {
            }
            template<class U>
            make_array_helper(const make_array_helper<T, U>& other) 
                : size(other.size),
                  data(other.data) {
            }
            pointer address(reference value) const {
                return &value;
            }
            const_pointer address(const_reference value) const {
                return &value;
            }
            size_type max_size() const {
                return static_cast<std::size_t>(-1) / sizeof(Y);
            }
            pointer allocate(size_type count, const void* = 0) {
                std::size_t a1 = alignment_of<T>::value;
                std::size_t n1 = count * sizeof(Y) + a1 - 1;
                void*  p1 = ::operator new(n1 + size);
                char*  p2 = static_cast<char*>(p1) + n1;
                while (std::size_t(p2) % a1 != 0) {
                    p2--;
                }
                *data = reinterpret_cast<T*>(p2);
                return  reinterpret_cast<Y*>(p1);
            }
            void deallocate(pointer memory, size_type) {
                void* p1 = memory;
                ::operator delete(p1);
            }
            void construct(pointer memory, const Y& value) {
                void* p1 = memory;
                ::new(p1) Y(value);
            }
            void destroy(pointer memory) {
                memory->~Y();
            }
            template<typename U>
            bool operator==(const make_array_helper<T, U>& other) const {
                return true;
            }
            template<typename U>
            bool operator!=(const make_array_helper<T, U>& other) const {
                return !(*this == other); 
            }
        private:
            std::size_t size;
            T** data;
        };
    }
}

#endif
