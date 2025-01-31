// Boost.Polygon library point_data.hpp header file

//          Copyright 2008 Intel Corporation.
//          Copyright Simonson Lucanus 2008-2012.
//          Copyright Andrii Sydorchuk 2012-2012.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

// See http://www.boost.org for updates, documentation, and revision history.

#ifndef BOOST_POLYGON_POINT_DATA_HPP
#define BOOST_POLYGON_POINT_DATA_HPP

#include "isotropy.hpp"
#include "point_concept.hpp"

namespace boost {
namespace polygon {

template <typename T>
class point_data {
 public:
  typedef T coordinate_type;

  point_data() : coords_() {}

  point_data(coordinate_type x, coordinate_type y) {
    coords_[HORIZONTAL] = x;
    coords_[VERTICAL] = y;
  }

  explicit point_data(const point_data& that) {
    coords_[0] = that.coords_[0];
    coords_[1] = that.coords_[1];
  }

  point_data& operator=(const point_data& that) {
    coords_[0] = that.coords_[0];
    coords_[1] = that.coords_[1];
    return *this;
  }

  // TODO(asydorchuk): Deprecated.
  template <typename PointType>
  explicit point_data(const PointType& that) {
    *this = that;
  }

  // TODO(asydorchuk): Deprecated.
  template <typename PointType>
  point_data& operator=(const PointType& that) {
    assign(*this, that);
    return *this;
  }

  // TODO(asydorchuk): Deprecated.
  template <typename CT>
  point_data(const point_data<CT>& that) {
    coords_[HORIZONTAL] = (coordinate_type)that.x();
    coords_[VERTICAL] = (coordinate_type)that.y();
  }

  bool operator==(const point_data& that) const {
    return (coords_[0] == that.coords_[0]) &&
      (coords_[1] == that.coords_[1]);
  }

  bool operator!=(const point_data& that) const {
    return !(*this == that);
  }

  bool operator<(const point_data& that) const {
    return (coords_[0] < that.coords_[0]) ||
      ((coords_[0] == that.coords_[0]) &&
       (coords_[1] < that.coords_[1]));
  }

  bool operator<=(const point_data& that) const {
    return !(that < *this);
  }

  bool operator>(const point_data& that) const {
    return that < *this;
  }

  bool operator>=(const point_data& that) const {
    return !(*this < that);
  }

  coordinate_type get(orientation_2d orient) const {
    return coords_[orient.to_int()];
  }

  void set(orientation_2d orient, coordinate_type value) {
    coords_[orient.to_int()] = value;
  }

  coordinate_type x() const {
    return coords_[HORIZONTAL];
  }

  coordinate_type y() const {
    return coords_[VERTICAL];
  }

  point_data& x(coordinate_type value) {
    coords_[HORIZONTAL] = value;
    return *this;
  }

  point_data& y(coordinate_type value) {
    coords_[VERTICAL] = value;
    return *this;
  }

 private:
  coordinate_type coords_[2];
};


template <typename CType>
struct geometry_concept< point_data<CType> > {
  typedef point_concept type;
};
}  // polygon
}  // boost

#endif  // BOOST_POLYGON_POINT_DATA_HPP
