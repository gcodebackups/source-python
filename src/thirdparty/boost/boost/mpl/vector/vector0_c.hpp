
#ifndef BOOST_MPL_VECTOR_VECTOR0_C_HPP_INCLUDED
#define BOOST_MPL_VECTOR_VECTOR0_C_HPP_INCLUDED

// Copyright Aleksey Gurtovoy 2000-2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: vector0_c.hpp 49239 2008-10-10 09:10:26Z agurtovoy $
// $Date: 2008-10-10 05:10:26 -0400 (Fri, 10 Oct 2008) $
// $Revision: 49239 $

#include <boost/mpl/vector/vector0.hpp>
#include <boost/mpl/integral_c.hpp>

namespace boost { namespace mpl {

template< typename T > struct vector0_c
    : vector0<>
{
    typedef vector0_c type;
    typedef T value_type;
};

}}

#endif // BOOST_MPL_VECTOR_VECTOR0_C_HPP_INCLUDED
