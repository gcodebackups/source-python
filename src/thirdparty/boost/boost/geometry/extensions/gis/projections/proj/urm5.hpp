#ifndef BOOST_GEOMETRY_PROJECTIONS_URM5_HPP
#define BOOST_GEOMETRY_PROJECTIONS_URM5_HPP

// Boost.Geometry - extensions-gis-projections (based on PROJ4)
// This file is automatically generated. DO NOT EDIT.

// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Original copyright notice:
 
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/extensions/gis/projections/impl/base_static.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_dynamic.hpp>
#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>
#include <boost/geometry/extensions/gis/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry { namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace urm5{ 

            struct par_urm5
            {
                double m, rmn, q3, n;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename Geographic, typename Cartesian, typename Parameters>
            struct base_urm5_spheroid : public base_t_f<base_urm5_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>
            {

                 typedef double geographic_type;
                 typedef double cartesian_type;

                par_urm5 m_proj_parm;

                inline base_urm5_spheroid(const Parameters& par)
                    : base_t_f<base_urm5_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>(*this, par) {}

                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    double t;
                
                    t = lp_lat = aasin(this->m_proj_parm.n * sin(lp_lat));
                    xy_x = this->m_proj_parm.m * lp_lon * cos(lp_lat);
                    t *= t;
                    xy_y = lp_lat * (1. + t * this->m_proj_parm.q3) * this->m_proj_parm.rmn;
                }
            };

            // Urmaev V
            template <typename Parameters>
            void setup_urm5(Parameters& par, par_urm5& proj_parm)
            {
                double alpha, t;
                proj_parm.n = pj_param(par.params, "dn").f;
                proj_parm.q3 = pj_param(par.params, "dq").f / 3.;
                alpha = pj_param(par.params, "ralpha").f;
                t = proj_parm.n * sin(alpha);
                proj_parm.m = cos(alpha) / sqrt(1. - t * t);
                proj_parm.rmn = 1. / (proj_parm.m * proj_parm.n);
                par.es = 0.;
                // par.inv = 0;
                // par.fwd = s_forward;
            }

        }} // namespace detail::urm5
    #endif // doxygen 

    /*!
        \brief Urmaev V projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Pseudocylindrical
         - Spheroid
         - n= q= alphi=
        \par Example
        \image html ex_urm5.gif
    */
    template <typename Geographic, typename Cartesian, typename Parameters = parameters>
    struct urm5_spheroid : public detail::urm5::base_urm5_spheroid<Geographic, Cartesian, Parameters>
    {
        inline urm5_spheroid(const Parameters& par) : detail::urm5::base_urm5_spheroid<Geographic, Cartesian, Parameters>(par)
        {
            detail::urm5::setup_urm5(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Factory entry(s)
        template <typename Geographic, typename Cartesian, typename Parameters>
        class urm5_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>
        {
            public :
                virtual projection<Geographic, Cartesian>* create_new(const Parameters& par) const
                {
                    return new base_v_f<urm5_spheroid<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);
                }
        };

        template <typename Geographic, typename Cartesian, typename Parameters>
        inline void urm5_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)
        {
            factory.add_to_factory("urm5", new urm5_entry<Geographic, Cartesian, Parameters>);
        }

    } // namespace detail 
    #endif // doxygen

}}} // namespace boost::geometry::projections

#endif // BOOST_GEOMETRY_PROJECTIONS_URM5_HPP

