// Copyright (C) 2001-2003
// William E. Kempf
// Copyright (C) 2011-2012 Vicente J. Botet Escriba
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_THREAD_CONFIG_WEK01032003_HPP
#define BOOST_THREAD_CONFIG_WEK01032003_HPP

// Force SIG_ATOMIC_MAX to be defined
//#ifndef __STDC_LIMIT_MACROS
//#define __STDC_LIMIT_MACROS
//#endif

#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>
#include <boost/thread/detail/platform.hpp>

#if ! defined BOOST_THREAD_NOEXCEPT_OR_THROW
#ifdef BOOST_NO_NOEXCEPT
#  define BOOST_THREAD_NOEXCEPT_OR_THROW throw()
#else
#  define BOOST_THREAD_NOEXCEPT_OR_THROW noexcept
#endif
#endif

#if defined BOOST_THREAD_THROW_IF_PRECONDITION_NOT_SATISFIED
#define BOOST_THREAD_ASSERT_PRECONDITION(EXPR, EX) \
        if (EXPR) {} else boost::throw_exception(EX)
#define BOOST_THREAD_VERIFY_PRECONDITION(EXPR, EX) \
        if (EXPR) {} else boost::throw_exception(EX)
#define BOOST_THREAD_THROW_ELSE_RETURN(EX, RET) \
        boost::throw_exception(EX)
#else
#define BOOST_THREAD_ASSERT_PRECONDITION(EXPR, EX)
#define BOOST_THREAD_VERIFY_PRECONDITION(EXPR, EX) \
        (void)(EXPR)
#define BOOST_THREAD_THROW_ELSE_RETURN(EX, RET) \
        return (RET)
#endif

// This compiler doesn't support Boost.Chrono
#if defined __IBMCPP__ && (__IBMCPP__ < 1100) && ! defined BOOST_THREAD_DONT_USE_CHRONO
#define BOOST_THREAD_DONT_USE_CHRONO
#define BOOST_THREAD_USE_DATE
#endif

// This compiler doesn't support Boost.Move
#if BOOST_WORKAROUND(__SUNPRO_CC, < 0x5100) && ! defined BOOST_THREAD_DONT_USE_MOVE
#define BOOST_THREAD_DONT_USE_MOVE
#endif

// This compiler doesn't support Boost.Container Allocators files
#if defined __SUNPRO_CC && ! defined BOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#define BOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#endif

#if defined _WIN32_WCE && _WIN32_WCE==0x501 && ! defined BOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#define BOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS
#endif

#if ! defined BOOST_THREAD_DONT_PROVIDE_BASIC_THREAD_ID && ! defined BOOST_THREAD_PROVIDES_BASIC_THREAD_ID
#define BOOST_THREAD_PROVIDES_BASIC_THREAD_ID
#endif

#if defined BOOST_NO_CXX11_RVALUE_REFERENCES || defined BOOST_MSVC
#define BOOST_THREAD_RVALUE_REFERENCES_DONT_MATCH_FUNTION_PTR
#endif


// Default version is 3
#if !defined BOOST_THREAD_VERSION
#define BOOST_THREAD_VERSION 2
#else
#if BOOST_THREAD_VERSION!=2  && BOOST_THREAD_VERSION!=3 && BOOST_THREAD_VERSION!=4
#error "BOOST_THREAD_VERSION must be 2, 3 or 4"
#endif
#endif

// CHRONO
// Uses Boost.Chrono by default if not stated the opposite defining BOOST_THREAD_DONT_USE_CHRONO
#if ! defined BOOST_THREAD_DONT_USE_CHRONO && ! defined BOOST_THREAD_USES_CHRONO
#define BOOST_THREAD_USES_CHRONO
#endif

#if BOOST_THREAD_VERSION==2

// PROVIDE_PROMISE_LAZY
#if ! defined BOOST_THREAD_DONT_PROVIDE_PROMISE_LAZY \
  && ! defined BOOST_THREAD_PROVIDES_PROMISE_LAZY
#define BOOST_THREAD_PROVIDES_PROMISE_LAZY
#endif

// PROVIDE_THREAD_EQ
#if ! defined BOOST_THREAD_DONT_PROVIDE_THREAD_EQ \
  && ! defined BOOST_THREAD_PROVIDES_THREAD_EQ
#define BOOST_THREAD_PROVIDES_THREAD_EQ
#endif

#endif

#if BOOST_THREAD_VERSION>=3

// ONCE_CXX11
#if ! defined BOOST_THREAD_DONT_PROVIDE_ONCE_CXX11 \
 && ! defined BOOST_THREAD_PROVIDES_ONCE_CXX11
#define BOOST_THREAD_PROVIDES_ONCE_CXX11
#endif

// THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE
#if ! defined BOOST_THREAD_DONT_PROVIDE_THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE \
 && ! defined BOOST_THREAD_PROVIDES_THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE
#define BOOST_THREAD_PROVIDES_THREAD_DESTRUCTOR_CALLS_TERMINATE_IF_JOINABLE
#endif

// THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE
#if ! defined BOOST_THREAD_DONT_PROVIDE_THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE \
 && ! defined BOOST_THREAD_PROVIDES_THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE
#define BOOST_THREAD_PROVIDES_THREAD_MOVE_ASSIGN_CALLS_TERMINATE_IF_JOINABLE
#endif

// PROVIDE_FUTURE
#if ! defined BOOST_THREAD_DONT_PROVIDE_FUTURE \
 && ! defined BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE
#endif

// FUTURE_CTOR_ALLOCATORS
#if ! defined BOOST_THREAD_DONT_PROVIDE_FUTURE_CTOR_ALLOCATORS \
 && ! defined BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#define BOOST_THREAD_PROVIDES_FUTURE_CTOR_ALLOCATORS
#endif

// SHARED_MUTEX_UPWARDS_CONVERSIONS
#if ! defined BOOST_THREAD_DONT_PROVIDE_SHARED_MUTEX_UPWARDS_CONVERSIONS \
 && ! defined BOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#define BOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#endif

// PROVIDE_EXPLICIT_LOCK_CONVERSION
#if ! defined BOOST_THREAD_DONT_PROVIDE_EXPLICIT_LOCK_CONVERSION \
 && ! defined BOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
#define BOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
#endif

// GENERIC_SHARED_MUTEX_ON_WIN
#if ! defined BOOST_THREAD_DONT_PROVIDE_GENERIC_SHARED_MUTEX_ON_WIN \
 && ! defined BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#define BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#endif

// USE_MOVE
#if ! defined BOOST_THREAD_DONT_USE_MOVE \
 && ! defined BOOST_THREAD_USES_MOVE
#define BOOST_THREAD_USES_MOVE
#endif

#endif

// deprecated since version 4
#if BOOST_THREAD_VERSION < 4

// NESTED_LOCKS
#if ! defined BOOST_THREAD_PROVIDES_NESTED_LOCKS \
 && ! defined BOOST_THREAD_DONT_PROVIDE_NESTED_LOCKS
#define BOOST_THREAD_PROVIDES_NESTED_LOCKS
#endif

// CONDITION
#if ! defined BOOST_THREAD_PROVIDES_CONDITION \
 && ! defined BOOST_THREAD_DONT_PROVIDE_CONDITION
#define BOOST_THREAD_PROVIDES_CONDITION
#endif

// USE_DATETIME
#if ! defined BOOST_THREAD_DONT_USE_DATETIME \
 && ! defined BOOST_THREAD_USES_DATETIME
#define BOOST_THREAD_USES_DATETIME
#endif
#endif

#if BOOST_THREAD_VERSION>=4

// SIGNATURE_PACKAGED_TASK
#if ! defined BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK \
 && ! defined BOOST_THREAD_DONT_PROVIDE_SIGNATURE_PACKAGED_TASK
#define BOOST_THREAD_PROVIDES_SIGNATURE_PACKAGED_TASK
#endif

// VARIADIC_THREAD
#if ! defined BOOST_THREAD_PROVIDES_VARIADIC_THREAD \
 && ! defined BOOST_THREAD_DONT_PROVIDE_VARIADIC_THREAD

#if ! defined(BOOST_NO_SFINAE_EXPR) && \
    ! defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES) && \
    ! defined(BOOST_NO_CXX11_DECLTYPE) && \
    ! defined(BOOST_NO_CXX11_DECLTYPE_N3276) && \
    ! defined(BOOST_NO_CXX11_AUTO) && \
    ! defined(BOOST_NO_CXX11_RVALUE_REFERENCES) && \
    ! defined(BOOST_NO_CXX11_HDR_TUPLE)

#define BOOST_THREAD_PROVIDES_VARIADIC_THREAD
#endif
#endif

// FUTURE_CONTINUATION
#if ! defined BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION \
 && ! defined BOOST_THREAD_DONT_PROVIDE_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#endif

// FUTURE_INVALID_AFTER_GET
#if ! defined BOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET \
 && ! defined BOOST_THREAD_DONT_PROVIDE_FUTURE_INVALID_AFTER_GET
#define BOOST_THREAD_PROVIDES_FUTURE_INVALID_AFTER_GET
#endif

// NESTED_LOCKS
#if ! defined BOOST_THREAD_PROVIDES_NESTED_LOCKS \
 && ! defined BOOST_THREAD_DONT_PROVIDE_NESTED_LOCKS
#define BOOST_THREAD_DONT_PROVIDE_NESTED_LOCKS
#endif

// CONDITION
#if ! defined BOOST_THREAD_PROVIDES_CONDITION \
 && ! defined BOOST_THREAD_DONT_PROVIDE_CONDITION
#define BOOST_THREAD_DONT_PROVIDE_CONDITION
#endif

#endif // BOOST_THREAD_VERSION>=4

// INTERRUPTIONS
#if ! defined BOOST_THREAD_PROVIDES_INTERRUPTIONS \
 && ! defined BOOST_THREAD_DONT_PROVIDE_INTERRUPTIONS
#define BOOST_THREAD_PROVIDES_INTERRUPTIONS
#endif

// CORRELATIONS

// EXPLICIT_LOCK_CONVERSION.
#if defined BOOST_THREAD_PROVIDES_EXPLICIT_LOCK_CONVERSION
#define BOOST_THREAD_EXPLICIT_LOCK_CONVERSION explicit
#else
#define BOOST_THREAD_EXPLICIT_LOCK_CONVERSION
#endif

// BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN is defined if BOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS
#if defined BOOST_THREAD_PROVIDES_SHARED_MUTEX_UPWARDS_CONVERSIONS \
&& ! defined BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#define BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
#endif

// BOOST_THREAD_PROVIDES_DEPRECATED_FEATURES_SINCE_V3_0_0 defined by default up to Boost 1.52
// BOOST_THREAD_DONT_PROVIDE_DEPRECATED_FEATURES_SINCE_V3_0_0 defined by default up to Boost 1.55
#if defined BOOST_THREAD_PROVIDES_DEPRECATED_FEATURES_SINCE_V3_0_0

#if  ! defined BOOST_THREAD_PROVIDES_THREAD_EQ
#define BOOST_THREAD_PROVIDES_THREAD_EQ
#endif

//#if ! defined BOOST_THREAD_USES_DATETIME
//#define defined BOOST_THREAD_USES_DATETIME
//#endif

#endif

#if BOOST_WORKAROUND(__BORLANDC__, < 0x600)
#  pragma warn -8008 // Condition always true/false
#  pragma warn -8080 // Identifier declared but never used
#  pragma warn -8057 // Parameter never used
#  pragma warn -8066 // Unreachable code
#endif

#include <boost/thread/detail/platform.hpp>

// provided for backwards compatibility, since this
// macro was used for several releases by mistake.
#if defined(BOOST_THREAD_DYN_DLL) && ! defined BOOST_THREAD_DYN_LINK
# define BOOST_THREAD_DYN_LINK
#endif

// compatibility with the rest of Boost's auto-linking code:
#if defined(BOOST_THREAD_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)
# undef  BOOST_THREAD_USE_LIB
# define BOOST_THREAD_USE_DLL
#endif

#if defined(BOOST_THREAD_BUILD_DLL)   //Build dll
#elif defined(BOOST_THREAD_BUILD_LIB) //Build lib
#elif defined(BOOST_THREAD_USE_DLL)   //Use dll
#elif defined(BOOST_THREAD_USE_LIB)   //Use lib
#else //Use default
#   if defined(BOOST_THREAD_PLATFORM_WIN32)
#       if defined(BOOST_MSVC) || defined(BOOST_INTEL_WIN)
            //For compilers supporting auto-tss cleanup
            //with Boost.Threads lib, use Boost.Threads lib
#           define BOOST_THREAD_USE_LIB
#       else
            //For compilers not yet supporting auto-tss cleanup
            //with Boost.Threads lib, use Boost.Threads dll
#           define BOOST_THREAD_USE_DLL
#       endif
#   else
#       define BOOST_THREAD_USE_LIB
#   endif
#endif

#if defined(BOOST_HAS_DECLSPEC)
#   if defined(BOOST_THREAD_BUILD_DLL) //Build dll
#       define BOOST_THREAD_DECL BOOST_SYMBOL_EXPORT
//#       define BOOST_THREAD_DECL __declspec(dllexport)

#   elif defined(BOOST_THREAD_USE_DLL) //Use dll
#       define BOOST_THREAD_DECL BOOST_SYMBOL_IMPORT
//#       define BOOST_THREAD_DECL __declspec(dllimport)
#   else
#       define BOOST_THREAD_DECL
#   endif
#elif (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  define BOOST_THREAD_DECL BOOST_SYMBOL_VISIBLE

#else
#   define BOOST_THREAD_DECL
#endif // BOOST_HAS_DECLSPEC

//
// Automatically link to the correct build variant where possible.
//
#if !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_THREAD_NO_LIB) && !defined(BOOST_THREAD_BUILD_DLL) && !defined(BOOST_THREAD_BUILD_LIB)
//
// Tell the autolink to link dynamically, this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#if defined(BOOST_THREAD_USE_DLL)
#   define BOOST_DYN_LINK
#endif
//
// Set the name of our library, this will get undef'ed by auto_link.hpp
// once it's done with it:
//
#if defined(BOOST_THREAD_LIB_NAME)
#    define BOOST_LIB_NAME BOOST_THREAD_LIB_NAME
#else
#    define BOOST_LIB_NAME boost_thread
#endif
//
// If we're importing code from a dll, then tell auto_link.hpp about it:
//
// And include the header that does the work:
//
#include <boost/config/auto_link.hpp>
#endif  // auto-linking disabled

#endif // BOOST_THREAD_CONFIG_WEK1032003_HPP

// Change Log:
//   22 Jan 05 Roland Schwarz (speedsnail)
//      Usage of BOOST_HAS_DECLSPEC macro.
//      Default again is static lib usage.
//      BOOST_DYN_LINK only defined when autolink included.
