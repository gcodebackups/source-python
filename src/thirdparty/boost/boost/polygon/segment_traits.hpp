// Boost.Polygon library segment_traits.hpp header file

//          Copyright 2008 Intel Corporation.
//          Copyright Simonson Lucanus 2008-2012.
//          Copyright Andrii Sydorchuk 2012-2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org for updates, documentation, and revision history.

#ifndef BOOST_POLYGON_SEGMENT_TRAITS_HPP
#define BOOST_POLYGON_SEGMENT_TRAITS_HPP

#include "isotropy.hpp"

namespace boost {
namespace polygon {

template <typename Segment>
struct segment_traits {
  typedef Segment segment_type;
  typedef typename segment_type::point_type point_type;
  typedef typename segment_type::coordinate_type coordinate_type;

  static point_type get(
      const segment_type& segment, direction_1d dir) {
    return segment.get(dir);
  }
};

template <typename Segment>
struct segment_mutable_traits {
  typedef Segment segment_type;
  typedef typename segment_type::point_type point_type;
  typedef typename segment_type::coordinate_type coordinate_type;

  static void set(
      segment_type& segment, direction_1d dir, const point_type& point) {
    segment.set(dir, point);
  }

  static segment_type construct(const point_type& low, const point_type& high) {
    return segment_type(low, high);
  }
};
}  // polygon
}  // boost

#endif  // BOOST_POLYGON_SEGMENT_TRAITS_HPP
