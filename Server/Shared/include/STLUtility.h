/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STLUTILITY_H
#define STLUTILITY_H

#include "DeleteHelpers.h"

#include <iterator>
#include <functional>
#include <numeric>

#include "MC2String.h"

namespace std {
   /**
    * This operator<< that prints pairs of arbitrary types is inserted
    * into the std namespace so that the lookup will work properly.
    */
   template<class T, class U>
   ostream& operator<<( ostream& stream, const pair<T,U>& p )
   {
      return stream << "(" << p.first << "," << p.second << ")";
   }
}

/// contains usefull STL utilities
namespace STLUtility {
/**
 * Returns true if container has \c key.
 * This is a more expressive way than to use the:
 * \code
 * if ( container.find( key ) != container.end() ) {
 *    // .. do stuff ..
 * }
 * \endcode
 * compare this to:
 * \code
 * if ( has( container, key ) ) {
 *   // .. do stuff ..
 * }
 * \endcode
 */
template < typename Container, typename Key >
bool has( const Container& other, const Key& key ) {
   return other.find( key ) != other.end();
}
/// Specalization of MC2String has
template < typename Key >
bool has( const MC2String& other, const Key& key ) {
   return other.find( key ) != MC2String::npos;
}

/// function that clears by swaping
template< typename T >
void swapClear( T& toClear ) {
   T().swap( toClear );
}

/// Helper class for sorting pointers in vector.
struct RefLess {
   template <typename Ptr>
   bool operator ()( const Ptr* first,
                     const Ptr* second ) const {
      // redirect to the real comparision function
      return *first < *second;
   }
};

/// Helper class for comparing objects in vector of pointers
// Dereferences the pointer and uses == to compare the objects
template <typename T>
struct RefEqualCmp {
   explicit RefEqualCmp( const T& obj ):
      m_obj( obj ) { }

   bool operator () ( const T* ptr ) const {
      return m_obj == *ptr;
   }
   const T m_obj;
};

/**
 * accumulator callback functor
 * adds result from a member fuction
 *
 */
template <typename Result, typename Class>
struct accumulate_cb_t {
   typedef Class argument_type;
   typedef Result result_type;

   accumulate_cb_t(std::const_mem_fun_t<Result, Class> f):m_func(f) {}
   /// calls member function and adds it to sum
   result_type operator ()(result_type sum, 
                           const argument_type* type ) const {
      return sum += m_func(type);
   }
   result_type operator ()(result_type sum, 
                           const argument_type& type ) const {
      return sum += m_func(&type);
   }
private:
   std::const_mem_fun_t<Result, Class> m_func;
};

/**
 * @name Helper functions for accumulate_cb_t
 * Example: 
 * int sum = accumulate( array.begin(), array.end(), 0,
 *                       accumumulate_cb(mem_fun(&ArrayItem::func) ) );
 * (if func returns type int)
 */
//@{
template <typename Result, typename Class>
accumulate_cb_t<Result, Class>
accumulate_cb(std::const_mem_fun_t<Result, Class> f ) {
   return accumulate_cb_t<Result, Class>( f );
}

template <typename Result, typename Class>
accumulate_cb_t<Result, Class>
accumulate_cb(Result (Class::*f)() const)
{
   return accumulate_cb_t<Result,Class>( std::mem_fun( f ) );
}

template <typename Result, typename Class>
accumulate_cb_t<Result, Class>
accumulate_cb(Result (Class::*f)())
{
   return accumulate_cb_t<Result,Class>( std::mem_fun( f ) );
}
//@}

/**
 *    Iterator dumper. <br />
 *    Class for dumping stuff to mc2dbg8 etc.
 *    Needed since they are defined as if ( false )
 *    Use like so: mc2dbg << it_dump( cont.begin(), cont.end(), ",") << endl;
 */
template<class ITERATOR>
class it_dumper {
public:
  
   it_dumper( const ITERATOR& begin,
              const ITERATOR& end,
              const char* sep = " ") :
         m_begin(begin),
         m_end(end),
         m_sep(sep) {}

   friend ostream& operator<<( ostream& o, const it_dumper& dumper ) {
      const char* cursep = "";
      for ( ITERATOR i = dumper.m_begin; i != dumper.m_end; ++i ) {
         o << cursep << *i;
         cursep = dumper.m_sep;
      }
      return o;
   }
   
private:
   ITERATOR m_begin;
   ITERATOR m_end;
   const char*     m_sep;
};

template<class ITERATOR>
it_dumper<ITERATOR> it_dump( const ITERATOR& begin,
                             const ITERATOR& end,
                             const char* sep = " " ) {
   return it_dumper<ITERATOR>( begin, end, sep );
}
   
/**
 *    Container dumper. <br />
 *    Class for dumping stuff to mc2dbg8 etc.
 *    Needed since they are defined as if ( false )
 *    Use like so:
 *    vector<uint32> cont;
 *    mc2dbg << co_dump( cont, ",") << endl;
 */
template<class CONTAINER>
class co_dumper {
public:

   co_dumper( const CONTAINER& container,
              const char* sep = " " ) : m_cont( container ),
                                        m_sep( sep ) {
   }
   
   friend ostream& operator<<( ostream& o, const co_dumper& dumper ) {
      const char* cursep = "";
      for ( typename CONTAINER::const_iterator i = dumper.m_cont.begin();
            i != dumper.m_cont.end();
            ++i ) {
         o << cursep << *i;
         cursep = dumper.m_sep;
      }
      return o;
   }
private:
   const CONTAINER& m_cont;
   const char*      m_sep;
   
};

template<class CONTAINER>
co_dumper<CONTAINER> co_dump( const CONTAINER& container,
                              const char* sep = " " )
{
   return co_dumper<CONTAINER>( container, sep );
}

/**
 * Iterator that extends over an array and a second container
 *
 */
template <typename A, typename B>
struct DualForwardIterator: 
      public std::iterator<std::input_iterator_tag, A> {

   typedef B SecondBlockIterator;
   typedef A* FirstBlockIterator;

   DualForwardIterator(FirstBlockIterator firstBlockBegin, 
                       FirstBlockIterator firstBlockEnd, 
                       SecondBlockIterator secondBlockBegin ):
      m_firstBlock( firstBlockBegin ),
      m_firstBlockEnd( firstBlockEnd ),
      m_secondBlock( secondBlockBegin ) { 
   }

   DualForwardIterator<A,B>& operator ++() {
      if ( m_firstBlock == m_firstBlockEnd ) {
         ++m_secondBlock;
      } else {
         ++m_firstBlock;
      }
      
      return *this;
   }

   DualForwardIterator<A,B> operator ++(int) {
      DualForwardIterator<A, B> tmp( *this );

      if ( m_firstBlock == m_firstBlockEnd ) {
         ++m_secondBlock;
      } else {
         ++m_firstBlock;
      }
      
      return tmp;
   }

   bool operator == (const DualForwardIterator<A, B>& comp ) const {
      return 
         comp.m_firstBlock == m_firstBlock &&
         comp.m_secondBlock == m_secondBlock;
   }

   bool operator != ( const DualForwardIterator<A, B>& comp ) const {
      return ! ( *this == comp );
   }

   A& operator *() {

      if ( m_firstBlock != m_firstBlockEnd ) {
         return *m_firstBlock;         
      }

      return *(*m_secondBlock);
   }

   const A& operator *() const {

      if ( m_firstBlock != m_firstBlockEnd ) {
         return *m_firstBlock;
      }

      return *(*m_secondBlock);
   }
private:
   FirstBlockIterator m_firstBlock;
   const FirstBlockIterator m_firstBlockEnd;
   SecondBlockIterator m_secondBlock;
};

/// If then else. Selects first type if bool is true.
template<bool, typename Ta, typename Tb>
class IfThenElse;

template<typename Ta, typename Tb>
class IfThenElse<true, Ta, Tb> {
public:
   typedef Ta ResultT;
};

template<typename Ta, typename Tb>
class IfThenElse<false, Ta, Tb> {
public:
   typedef Tb ResultT;
};

#define IF_THEN_ELSE(a,b,c) typename STLUtility::IfThenElse<a,b,c>::ResultT

/// selects the second value from a pair
template <typename Pair>
struct select2nd : public unary_function<Pair,typename Pair::second_type>
{
   const typename Pair::second_type& operator() ( const Pair& p ) const {
      return p.second;
   }
   typename  Pair::second_type& operator() (  Pair& p ) const {
      return p.second;
   }
};

/// Use this to determine select2nd type, instead of explicitly typing name.
/// Example: 
/// \code
/// map<key, value> values; 
/// // old style:
/// transform( values.begin(), values.end(), 
///            someothercontainer.begin(), select2nd< map< key, value > >() );
/// // new style:
/// transform( values.begin(), values.end(),
///            someothercontainer.begin(), makeSelect2nd( values ) );
/// \endcode
template <typename T>
select2nd<typename T::value_type> makeSelect2nd( const T& container ) {
   return select2nd<typename T::value_type>();
}

/// selects the first value from a pair
template <typename Pair>
struct select1st : public unary_function<Pair,typename Pair::first_type>
{
   const typename Pair::first_type& operator() ( const Pair& p ) const {
      return p.first;
   }
   typename  Pair::second_type& operator() (  Pair& p ) const {
      return p.first;
   }
};


/// returns the same value
template <typename T>
struct identity: public unary_function< T, T > {
   T& operator()( T& t ) {
      return t;
   }
   const T& operator()( const T& t ) const {
      return t;
   }
};

template <typename T>
identity<typename T::value_type> makeIdentity( const T& val ) {
   return identity< typename T::value_type >();
}

// generator
template <typename T>
class Counter {
public:
   Counter(T start):m_counter(start) { }
   T operator()() {
      //Shall be postfix so that the return value is correct.
      return m_counter++;
   }
private:
   T m_counter;
};

/**
 * IO-stream maipulator for formatted time printing. Uses strftime for
 * the actual formatting.
 */
class printTime
{
   /** The format to print in. */
   const MC2String m_format;
   /** The time to format. */
   time_t m_time;
public:
   /**
    * Constructor.
    * @param time The time_t value that should be printed according to
    *             the format specifier.
    * @param format The strftime format that should be used. Default
    *               value is "%+" which is "The date and time in
    *               date(1) format.".
    */
   printTime( time_t time, const MC2String format = "%FT%T%z" ) : 
      m_format(format), m_time(time)
   {}
   /**
    * Output operator for printTime objects. 
    * @str The ostream to print on. 
    * @pt The printTime object.
    * @return The used ostream. 
    */
   friend ostream& operator<<(ostream& str, const printTime& pt)
   {
      struct tm split = {0};
      localtime_r( &(pt.m_time), &split );
      char buf[32] = "";
      if( 0 == strftime( buf, sizeof(buf), pt.m_format.c_str(), &split ) ){
         *buf = 0;
      }
      return str << buf;
   }
};

}
#endif 
