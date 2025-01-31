// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_DMS_PARSER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_DMS_PARSER_HPP

// This file is totally revised from PROJ4 dmstor.c

// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Geometry Library by Barend Gehrels (Geodan, Amsterdam)

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

#include <string>

#include <boost/static_assert.hpp>

#if !defined(BOOST_GEOMETRY_NO_LEXICAL_CAST)
#include <boost/lexical_cast.hpp>
#endif // !defined(BOOST_GEOMETRY_NO_LEXICAL_CAST)

#include <boost/algorithm/string.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/extensions/strategies/parse.hpp>

#include <boost/geometry/util/math.hpp>

namespace boost { namespace geometry
{


struct dms_result
{
    enum axis_selector {axis_lat = 1, axis_lon = 0};

    private :
        typedef double T;
        T m_angle;
        axis_selector m_axis;

    public :

        explicit dms_result(T const& v, axis_selector ax)
            : m_angle(v)
            , m_axis(ax)
        {}

        inline axis_selector axis() const { return m_axis; }

        inline operator double() const { return m_angle; }

        template <typename CH, typename TR>
        inline friend std::basic_ostream<CH, TR>& operator<<(std::basic_ostream<CH, TR>& os,
                        const dms_result& d)
        {
            os << d.m_angle;
            return os;
        }

};


namespace strategy
{

    template <bool as_radian = true
            , char N = 'N', char E = 'E', char S = 'S', char W = 'W' // translatable
            , char MIN = '\'', char SEC = '"' // other char's possible
            , char D = 'D', char R = 'R' // degree sign might be small o
            >
    struct dms_parser
    {


        // Question from Barend: can we compile-time select that it is case-sensitive/case-insensitive?
        // We have to change the switch then -> specializations

        // For now: make it (compile-time) case sensitive
        static const int diff = 'a' - 'A';
#ifndef __GNUC__
        BOOST_STATIC_ASSERT((diff > 0)); // make sure we've the right assumption. GCC does not accept this here.
#endif
        static const char n_alter = N <= 'Z' ? N + diff : N - diff;
        static const char e_alter = E <= 'Z' ? E + diff : E - diff;
        static const char s_alter = S <= 'Z' ? S + diff : S - diff;
        static const char w_alter = W <= 'Z' ? W + diff : W - diff;

        static const char r_alter = R <= 'Z' ? R + diff : R - diff;

        // degree is normally D (proj4) but might be superscript o
        // Note d_alter is not correct then, so map it to NULL now, guarded by the while
        static const char d_alter =
            ((D >= 'A' && D <= 'Z') || (D >= 'a' && D <= 'z')) ? (D <= 'Z' ? D + diff : D - diff) : '\0';


        struct dms_value
        {
            double dms[3];
            bool has_dms[3];

            dms_value()
            {
                memset(this, 0, sizeof(dms_value));
            }
        };


        template <size_t I>
        static inline void assign_dms(dms_value& dms, std::string& value, bool& has_value)
        {
#if !defined(BOOST_GEOMETRY_NO_LEXICAL_CAST)
            dms.dms[I] = boost::lexical_cast<double>(value.c_str());
#else // !defined(BOOST_GEOMETRY_NO_LEXICAL_CAST)
            dms.dms[I] = std::atof(value.c_str());
#endif // !defined(BOOST_GEOMETRY_NO_LEXICAL_CAST)
            dms.has_dms[I] = true;
            has_value = false;
            value.clear();
        }

        static inline void process(dms_value& dms, std::string& value, bool& has_value)
        {
            if (has_value)
            {
                // Assign last one, sequentially
                if (! dms.has_dms[0]) assign_dms<0>(dms, value, has_value);
                else if (! dms.has_dms[1]) assign_dms<1>(dms, value, has_value);
                else if (! dms.has_dms[2]) assign_dms<2>(dms, value, has_value);
            }
        }


        dms_result operator()(const char* is) const
        {
            dms_value dms;
            bool has_value = false;
            std::string value;

            double factor = 1.0; // + denotes N/E values, -1 denotes S/W values
            dms_result::axis_selector axis = dms_result::axis_lon; // true denotes N/S values
            bool in_radian = false; // true denotes values as "0.1R"

            while(*is)
            {
                switch(*is)
                {
                    case '-' :
                        if (! has_value && ! dms.has_dms[0])
                        {
                            factor = -factor;
                        }
                        break;
                    case N :
                    case n_alter :
                        axis = dms_result::axis_lat;
                        break;
                    case S :
                    case s_alter :
                        axis = dms_result::axis_lat;
                        factor = -factor;
                        break;
                    case E :
                    case e_alter :
                        axis = dms_result::axis_lon;
                        break;
                    case W :
                    case w_alter :
                        axis = dms_result::axis_lon;
                        factor = -factor;
                        break;
                    case D :
                    case d_alter :
                        if (! dms.has_dms[0] && has_value)
                        {
                            assign_dms<0>(dms, value, has_value);
                        }
                        break;
                    case R :
                    case r_alter :
                        if (! dms.has_dms[0] && has_value)
                        {
                            // specified value is in radian!
                            in_radian = true;
                            assign_dms<0>(dms, value, has_value);
                        }
                        break;
                    case MIN:
                        if (! dms.has_dms[1] && has_value)
                        {
                            assign_dms<1>(dms, value, has_value);
                        }
                        break;
                    case SEC :
                        if (! dms.has_dms[2] && has_value)
                        {
                            assign_dms<2>(dms, value, has_value);
                        }
                        break;
                    case ' ' :
                    case '\t' :
                    case '\n' :
                        process(dms, value, has_value);
                        break;
                    default :
                        value += *is;
                        has_value = true;
                        break;
                }
                is++;
            }

            // Assign last one, if any
            process(dms, value, has_value);

            return dms_result(factor *
                (in_radian && as_radian
                        ? dms.dms[0]
                : in_radian && ! as_radian
                        ? dms.dms[0] * math::r2d
                : ! in_radian && as_radian
                        ? dms.dms[0] * math::d2r + dms.dms[1] * math::d2r / 60.0 + dms.dms[2] * math::d2r / 3600.0
                        : dms.dms[0] + dms.dms[1] / 60.0 + dms.dms[2] / 3600.0)
                , axis);
        }
    };

}


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

template <template<typename> class CS>
struct strategy_parse<geographic_tag, CS<degree> >
{
    typedef strategy::dms_parser<false> type;
};


template <template<typename> class CS>
struct strategy_parse<geographic_tag, CS<radian> >
{
    typedef strategy::dms_parser<true> type;
};

#endif


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GIS_GEOGRAPHIC_STRATEGIES_DMS_PARSER_HPP
