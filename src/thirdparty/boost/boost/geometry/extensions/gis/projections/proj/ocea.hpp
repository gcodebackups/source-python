#ifndef BOOST_GEOMETRY_PROJECTIONS_OCEA_HPP
#define BOOST_GEOMETRY_PROJECTIONS_OCEA_HPP

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
    namespace detail { namespace ocea{ 

            struct par_ocea
            {
                double    rok;
                double    rtk;
                double    sinphi;
                double    cosphi;
                double    singam;
                double    cosgam;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename Geographic, typename Cartesian, typename Parameters>
            struct base_ocea_spheroid : public base_t_fi<base_ocea_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>
            {

                 typedef double geographic_type;
                 typedef double cartesian_type;

                par_ocea m_proj_parm;

                inline base_ocea_spheroid(const Parameters& par)
                    : base_t_fi<base_ocea_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>(*this, par) {}

                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    double t;
                
                    xy_y = sin(lp_lon);
                /*
                    xy_x = atan2((tan(lp_lat) * this->m_proj_parm.cosphi + this->m_proj_parm.sinphi * xy_y) , cos(lp_lon));
                */
                    t = cos(lp_lon);
                    xy_x = atan((tan(lp_lat) * this->m_proj_parm.cosphi + this->m_proj_parm.sinphi * xy_y) / t);
                    if (t < 0.)
                        xy_x += PI;
                    xy_x *= this->m_proj_parm.rtk;
                    xy_y = this->m_proj_parm.rok * (this->m_proj_parm.sinphi * sin(lp_lat) - this->m_proj_parm.cosphi * cos(lp_lat) * xy_y);
                }

                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    double t, s;
                
                    xy_y /= this->m_proj_parm.rok;
                    xy_x /= this->m_proj_parm.rtk;
                    t = sqrt(1. - xy_y * xy_y);
                    lp_lat = asin(xy_y * this->m_proj_parm.sinphi + t * this->m_proj_parm.cosphi * (s = sin(xy_x)));
                    lp_lon = atan2(t * this->m_proj_parm.sinphi * s - xy_y * this->m_proj_parm.cosphi,
                        t * cos(xy_x));
                }
            };

            // Oblique Cylindrical Equal Area
            template <typename Parameters>
            void setup_ocea(Parameters& par, par_ocea& proj_parm)
            {
                double phi_0=0.0, phi_1, phi_2, lam_1, lam_2, lonz, alpha;
                proj_parm.rok = par.a / par.k0;
                proj_parm.rtk = par.a * par.k0;
                if ( pj_param(par.params, "talpha").i) {
                    alpha    = pj_param(par.params, "ralpha").f;
                    lonz = pj_param(par.params, "rlonc").f;
                    proj_parm.singam = atan(-cos(alpha)/(-sin(phi_0) * sin(alpha))) + lonz;
                    proj_parm.sinphi = asin(cos(phi_0) * sin(alpha));
                } else {
                    phi_1 = pj_param(par.params, "rlat_1").f;
                    phi_2 = pj_param(par.params, "rlat_2").f;
                    lam_1 = pj_param(par.params, "rlon_1").f;
                    lam_2 = pj_param(par.params, "rlon_2").f;
                    proj_parm.singam = atan2(cos(phi_1) * sin(phi_2) * cos(lam_1) -
                        sin(phi_1) * cos(phi_2) * cos(lam_2),
                        sin(phi_1) * cos(phi_2) * sin(lam_2) -
                        cos(phi_1) * sin(phi_2) * sin(lam_1) );
                    proj_parm.sinphi = atan(-cos(proj_parm.singam - lam_1) / tan(phi_1));
                }
                par.lam0 = proj_parm.singam + HALFPI;
                proj_parm.cosphi = cos(proj_parm.sinphi);
                proj_parm.sinphi = sin(proj_parm.sinphi);
                proj_parm.cosgam = cos(proj_parm.singam);
                proj_parm.singam = sin(proj_parm.singam);
                // par.inv = s_inverse;
                // par.fwd = s_forward;
                par.es = 0.;
            }

        }} // namespace detail::ocea
    #endif // doxygen 

    /*!
        \brief Oblique Cylindrical Equal Area projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Cylindrical
         - Sph lonc= alpha= or
         - lat_1= lat_2= lon_1= lon_2=
        \par Example
        \image html ex_ocea.gif
    */
    template <typename Geographic, typename Cartesian, typename Parameters = parameters>
    struct ocea_spheroid : public detail::ocea::base_ocea_spheroid<Geographic, Cartesian, Parameters>
    {
        inline ocea_spheroid(const Parameters& par) : detail::ocea::base_ocea_spheroid<Geographic, Cartesian, Parameters>(par)
        {
            detail::ocea::setup_ocea(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Factory entry(s)
        template <typename Geographic, typename Cartesian, typename Parameters>
        class ocea_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>
        {
            public :
                virtual projection<Geographic, Cartesian>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<ocea_spheroid<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);
                }
        };

        template <typename Geographic, typename Cartesian, typename Parameters>
        inline void ocea_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)
        {
            factory.add_to_factory("ocea", new ocea_entry<Geographic, Cartesian, Parameters>);
        }

    } // namespace detail 
    #endif // doxygen

}}} // namespace boost::geometry::projections

#endif // BOOST_GEOMETRY_PROJECTIONS_OCEA_HPP

