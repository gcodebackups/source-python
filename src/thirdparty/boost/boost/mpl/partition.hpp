
#ifndef BOOST_MPL_PARTITION_HPP_INCLUDED
#define BOOST_MPL_PARTITION_HPP_INCLUDED

// Copyright Eric Friedman 2002-2003
// Copyright Aleksey Gurtovoy 2004
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at 
// http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/mpl for documentation.

// $Id: partition.hpp 49239 2008-10-10 09:10:26Z agurtovoy $
// $Date: 2008-10-10 05:10:26 -0400 (Fri, 10 Oct 2008) $
// $Revision: 49239 $

#include <boost/mpl/stable_partition.hpp>
#include <boost/mpl/aux_/inserter_algorithm.hpp>

namespace boost { namespace mpl {

namespace aux {
 
template <
      typename Sequence
    , typename Pred
    , typename In1
    , typename In2
    >
struct partition_impl
    : stable_partition_impl<Sequence,Pred,In1,In2>
{
};

template <
      typename Sequence
    , typename Pred
    , typename In1
    , typename In2
    >
struct reverse_partition_impl
    : reverse_stable_partition_impl<Sequence,Pred,In1,In2>
{
};

} // namespace aux

BOOST_MPL_AUX_INSERTER_ALGORITHM_DEF(4, partition)

}}

#endif // BOOST_MPL_PARTITION_HPP_INCLUDED
