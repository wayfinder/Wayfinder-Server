/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LOCALEUTILITY_H
#define LOCALEUTILITY_H

//decide whether to use c++ locales or GNU extension c-style locales.
//the c++ locales are standard, but not suuported on gcc < 3.
//the gnu extension locales are not standard, but have have some
//functionality that c++ locales don't have, such as strncmp.
#if defined (__GNUC__) && __GNUC__ > 2
//may already be defined in cpp file. 
# ifndef USE_CPP_LOCALES
#  define USE_CPP_LOCALES
# endif
#else
# undef  USE_CPP_LOCALES
#endif

#ifdef USE_CPP_LOCALES
# include <locale>
#endif

// Includes
#include "config.h"
#include "MC2String.h"
#include "LangTypes.h"

/**
 * Class that encapsulates the difference of using char*, MC2String,
 * or MC2String* as function arguments to any of the LocaleUtility
 *
 */
class LocaleStringRef {
public:
   /** 
    * @name Constructors. 
    * None of the constructors are explicit, that's the whole point.
    */
   //@{
   /**
    * Constructor from c-string.
    * @param str The string to use. NULL is considered to mean the
    *            empty string.
    */
   LocaleStringRef( const char* str );

   /**
    * Constructor from MC2String reference. 
    * @param str The string to use.
    */
   LocaleStringRef( const MC2String& str );

   /**
    * Constructor from MC2String pointer. 
    * @param str The string to use. NULL is considered to mean the
    *            empty string.
    */
   LocaleStringRef( const MC2String* str );

   //@}
   /** Gets the start of the string. */
   const char* begin() const;

   /** Gets the end of the string. */
   const char* end() const;

private:
   /** The start point of the string. */
   const char* const m_str;

   /**
    * The end point of the string. If the object was created from a
    * c-string, this pointer points to the \0-terminator. If it was
    * created from a MC2String it points one byte past the last
    * character (Mc2String::end()). Since it was computed from
    * MC2String::c_str it points to the \0-terminator even if created
    * from MC2String.
    */
   const char* const m_end;
};


/**
  *   Some useful locale related functions.
  *
  */
class LocaleUtility {
public:
      /**
       *    Explicit constructor using an LangType::language_t value
       *    to select locale.
       *    @param lang The language used to select the locale.
       */
      explicit LocaleUtility( LangTypes::language_t lang );

      /**
       *    Explicit constructor using a locale string to specify the
       *    locale. Should only be used when std::locale is supported.
       *    @param locale The locale as a string.
       */
      explicit LocaleUtility( const char* locale );

      /**
       * Copy constructor.
       */
      LocaleUtility( const LocaleUtility& o );

      /**
       * Destructor.
       */
      ~LocaleUtility();

      /**
       * Assignment constructor.
       */
      const LocaleUtility& operator=( const LocaleUtility& o );

      /**
        *   Compare two strings according to locale. Works like the
        *   system call strcmp (this method currently only pass the
        *   call to the system). 
        *
        *   Both arguments accept char*, MC2String, and MC2String& by
        *   implicit conversion to LocaleStringRef.
        *
        *   @param   strA  The first string to compare. 
        *   @param   strB  The second string to compare.
        *   @return  An integer less than 0 if strA < strB, 0 if 
        *            strA == strB and an integer > 0 if strA > strB.
        */
      int strcmp( const LocaleStringRef& strA, 
                  const LocaleStringRef& strB ) const;

      /**
       * Decide whether one string should preceed another in a collation. 
       *
       *   Both arguments accept char*, MC2String, and MC2String& by
       *   implicit conversion to LocaleStringRef.
       *
       * @param lhs The first string.
       * @param rhs The second string.
       * @return true if lhs should preceed rhs in the collation.
       */
      inline bool collate( const LocaleStringRef& lhs, 
                           const LocaleStringRef& rhs ) const;

      /**
       * Calls collate with the same arguments. This function makes it
       * possible to use this object as a ordering functor in string
       * containers.
       *
       *   Both arguments accept char*, MC2String, and MC2String& by
       *   implicit conversion to LocaleStringRef.
       *
       * @param lhs The left hand argument.
       * @param rhs The right hand argument.
       * @return lhs < rhs.
       */
      inline bool operator()( const LocaleStringRef& lhs, 
                              const LocaleStringRef& rhs ) const;
   
      /**
       * Make a date string.
       * 
       * @param t The UTC time.
       * @return A date string in locale.
       */
      MC2String makeDate( time_t t ) const;

      /**
       * Make a date string.
       * 
       * @param t The UTC time.
       * @return A date string in locale.
       */
      static MC2String makeDate( time_t t, LangTypes::language_t lang );

#if 0
   //this function was never completed.
      inline int strncmp(const char* strA, const char* strB, size_t n);
#endif


private:
#ifdef USE_CPP_LOCALES
      /** Standard C++ locale object.*/
      std::locale m_locale;
#else
      /** 
       * Pointer to GNU Extended locale object. 
       * Normally we would use the __locale_t typedef, but this way we
       * can treat include files better, we do not need to use the
       * _USE_GNU define in this header file, only in the cpp-file.
       */
      struct __locale_struct* m_locale;
#endif
};


class LocaleCompareLess {
public:
   explicit LocaleCompareLess( LangTypes::language_t language ):
      m_locale( language ) {
   }

   LocaleCompareLess( const LocaleCompareLess& other ):
      m_locale( other.m_locale ) {
      
   }

   bool operator() ( const LocaleStringRef& lhs, 
                     const LocaleStringRef& rhs ) const {
      return m_locale.collate( lhs, rhs );
   }
   
private:
   // we can not assign
   const LocaleCompareLess& operator = ( const LocaleCompareLess& );
   const LocaleUtility m_locale;
};

// -----------------------------------------------------------------------
//                                     Implementation of inlined methods -


inline bool 
LocaleUtility::collate( const LocaleStringRef& lhs, 
                        const LocaleStringRef& rhs) const
{
   return ( this->strcmp( lhs, rhs ) < 0 );
}

inline bool 
LocaleUtility::operator()( const LocaleStringRef& lhs, 
                           const LocaleStringRef& rhs ) const
{
   return this->collate( lhs, rhs );
}

inline 
LocaleStringRef::LocaleStringRef( const char* str ) 
      : m_str( str ? str : "" ), m_end( m_str + strlen(m_str) )
{
}

inline 
LocaleStringRef::LocaleStringRef( const MC2String& str )
      : m_str( str.c_str() ), m_end( m_str + str.length() )
{
}

inline 
LocaleStringRef::LocaleStringRef( const MC2String* str ) 
      : m_str( str ? str->c_str() : "" ), 
        m_end( m_str + (str ? str->length() : 0) )
{
}

inline const char*
LocaleStringRef::begin() const {
   return m_str;
}

inline const char*
LocaleStringRef::end() const {
   return m_end;
}


#endif // LOCALEUTILITY_H
