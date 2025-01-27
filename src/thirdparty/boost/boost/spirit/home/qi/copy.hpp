/*=============================================================================
    Copyright (c) 2001-2012 Joel de Guzman

    Distributed under the Boost Software License, Version 1.0. (See accompanying
    file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
==============================================================================*/
#if !defined(BOOST_SPIRIT_COPY_FEBRUARY_7_2012_0159PM)
#define BOOST_SPIRIT_COPY_FEBRUARY_7_2012_0159PM

#include <boost/config.hpp>
#include <boost/proto/proto.hpp>

#if defined(_MSC_VER)
#pragma once
#endif

#if !defined(BOOST_NO_AUTO_DECLARATIONS)

namespace boost { namespace spirit { namespace qi
{
    template <typename Expr>
    typename boost::proto::result_of::deep_copy<Expr>::type
    copy(Expr const& expr)
    {
        BOOST_SPIRIT_ASSERT_MATCH(boost::spirit::qi::domain, Expr);
        return boost::proto::deep_copy(expr);
    }
}}}

#endif
#endif
