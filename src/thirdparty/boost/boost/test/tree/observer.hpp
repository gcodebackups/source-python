//  (C) Copyright Gennadiy Rozental 2005-2012.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision: 81012 $
//
//  Description : defines abstract interface for test observer
// ***************************************************************************

#ifndef BOOST_TEST_TEST_OBSERVER_HPP_021005GER
#define BOOST_TEST_TEST_OBSERVER_HPP_021005GER

// Boost.Test
#include <boost/test/detail/fwd_decl.hpp>
#include <boost/test/detail/global_typedef.hpp>
#include <boost/test/detail/config.hpp>

#include <boost/test/detail/suppress_warnings.hpp>

//____________________________________________________________________________//

namespace boost {
namespace unit_test {

// ************************************************************************** //
// **************                 test_observer                ************** //
// ************************************************************************** //

class BOOST_TEST_DECL test_observer {
public:
    // test observer interface
    virtual void    test_start( counter_t /* test_cases_amount */ ) {}
    virtual void    test_finish() {}
    virtual void    test_aborted() {}

    virtual void    test_unit_start( test_unit const& ) {}
    virtual void    test_unit_finish( test_unit const&, unsigned long /* elapsed */ ) {}
    virtual void    test_unit_skipped( test_unit const& ) {}
    virtual void    test_unit_aborted( test_unit const& ) {}

    virtual void    assertion_result( bool /* passed */ ) {}
    virtual void    exception_caught( execution_exception const& ) {}

    virtual int     priority() { return 0; }

protected:
    BOOST_TEST_PROTECTED_VIRTUAL ~test_observer() {}
};

} // namespace unit_test
} // namespace boost

#include <boost/test/detail/enable_warnings.hpp>

#endif // BOOST_TEST_TEST_OBSERVER_HPP_021005GER

