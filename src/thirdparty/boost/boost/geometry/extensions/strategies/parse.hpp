// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_EXTENSIONS_PARSE_HPP
#define BOOST_GEOMETRY_STRATEGIES_EXTENSIONS_PARSE_HPP

#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{



/*!
    \brief Tagraits class binding a parsing strategy to a coordinate system
    \ingroup parse
    \tparam Tag tag of coordinate system of point-type
    \tparam CoordinateSystem coordinate system
*/
template <typename Tag, typename CoordinateSystem>
struct strategy_parse
{
    typedef strategy::not_implemented type;
};



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_EXTENSIONS_PARSE_HPP
