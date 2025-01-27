
//  (C) Copyright Edward Diener 2011
//  Use, modification and distribution are subject to the Boost Software License,
//  Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).

#if !defined(TTI_MF_TEMPLATE_HPP)
#define TTI_MF_TEMPLATE_HPP

#include <boost/config.hpp>
#include <boost/mpl/apply.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/quote.hpp>

/*

  The succeeding comments in this file are in doxygen format.

*/

/** \file
*/

namespace boost
  {
  namespace tti
    {
  
    /// A metafunction which checks whether a class template exists within an enclosing type.
    /**

        This metafunction takes its enclosing type as nullary metafunctions whose typedef 'type' member is the actual type used.
    
        template<class TTI_HasTemplate,class TTI_T>
        struct mf_has_template
          {
          static const value = unspecified;
          typedef mpl::bool_<true-or-false> type;
          };

        The metafunction types and return:

          TTI_HasTemplate = a Boost MPL lambda expression using the metafunction generated from the BOOST_TTI_HAS_TEMPLATE ( BOOST_TTI_TRAIT_HAS_TEMPLATE ) macro.<br />
                            The easiest way to generate the lambda expression is to use a Boost MPL placeholder expression of the form 'metafunction\<_\>'.
                            You can also use a metafunction class generated by boost::mpl::quote1.
                        
          TTI_T           = the enclosing type as a nullary metafunction.
      
          returns = 'value' is true if the template exists within the enclosing type,
                    otherwise 'value' is false.
                          
          The template being introspected must have all 'class' ( or 'typename' ) parameters types.
          
    */
    template
      <
      class TTI_HasTemplate,
      class TTI_T
      >
    struct mf_has_template :
      boost::mpl::apply
        <
        TTI_HasTemplate,
        typename TTI_T::type
        >::type 
      {
      };
    }
  }
  
#endif // TTI_MF_TEMPLATE_HPP
