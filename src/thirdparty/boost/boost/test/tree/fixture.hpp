//  (C) Copyright Gennadiy Rozental 2011-2012.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at 
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
//  File        : $RCSfile$
//
//  Version     : $Revision: 74640 $
//
//  Description : defines fixture interface and object makers
// ***************************************************************************

#ifndef BOOST_TEST_TREE_FIXTURE_HPP_100311GER
#define BOOST_TEST_TREE_FIXTURE_HPP_100311GER

// Boost.Test
#include <boost/test/detail/config.hpp>

// Boost
#include <boost/shared_ptr.hpp>
#include <boost/function/function0.hpp>

#include <boost/test/detail/suppress_warnings.hpp>

//____________________________________________________________________________//

namespace boost {
namespace unit_test {

// ************************************************************************** //
// **************               test_unit_fixture              ************** //
// ************************************************************************** //

class BOOST_TEST_DECL test_unit_fixture { 
public: 
    virtual ~test_unit_fixture() {}

    // Fixture interface
    virtual void    setup() = 0;
    virtual void    teardown() = 0;
}; 

typedef shared_ptr<test_unit_fixture> test_unit_fixture_ptr;

// ************************************************************************** //
// **************              class_based_fixture             ************** //
// ************************************************************************** //

template<typename F>
class class_based_fixture : public test_unit_fixture { 
public: 
    // Constructor
    class_based_fixture() : m_inst( 0 ) {}

private:
    // Fixture interface
    virtual void    setup()         { m_inst = new F; }
    virtual void    teardown()      { delete m_inst; }

    // Data members
    F*              m_inst;
}; 

// ************************************************************************** //
// **************            function_based_fixture            ************** //
// ************************************************************************** //

class function_based_fixture : public test_unit_fixture { 
public:
    // Constructor
    function_based_fixture( boost::function<void ()> const& setup, boost::function<void ()> const& teardown )
    : m_setup( setup )
    , m_teardown( teardown )
    {
    }

private:
    // Fixture interface
    virtual void    setup()         { if( m_setup ) m_setup(); }
    virtual void    teardown()      { if( m_teardown ) m_teardown(); }

    // Data members
    boost::function<void ()>    m_setup;
    boost::function<void ()>    m_teardown;
}; 

} // namespace unit_test
} // namespace boost

#include <boost/test/detail/enable_warnings.hpp>

#endif // BOOST_TEST_TREE_FIXTURE_HPP_100311GER

