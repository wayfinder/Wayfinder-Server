/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

//decide whether to use c++ locales or GNU extension c-style locales.
//the c++ locales are standard, but not suuported on gcc < 3.
//the gnu extension locales are not standard, but have have some
//functionality that c++ locales don't have, such as strncmp.
#if defined (__GNUC__) && __GNUC__ > 2
# define USE_CPP_LOCALES
#else
# undef  USE_CPP_LOCALES
#endif

#ifndef USE_CPP_LOCALES
//include features.h first, otherwise it turns off gnu extensions. 
# include <features.h>
//enable gnu extension
# define __USE_GNU
# include <locale.h>
# include <string.h>
# include <time.h>
# include <langinfo.h>
# undef __USE_GNU
//disable extansions.
#endif

#include "LocaleUtility.h"

namespace {
   const char* c_backupLocale = "en_GB.utf8";
}
#ifdef USE_CPP_LOCALES

#include <sstream>

namespace {
   std::locale GenerateLocale( const char* locale_str ) throw()
   {
      try {
         std::locale loc(locale_str);
         return loc;
      } 
      catch (...) {
         if(0 == strcmp(locale_str, c_backupLocale)){
            return std::locale();
         } else {
            return GenerateLocale(c_backupLocale);
         }
      }      
   }
}

LocaleUtility::LocaleUtility( LangTypes::language_t lang ) :
   m_locale( GenerateLocale( LangTypes::getUtf8LocaleAsString( lang )))
{
}


LocaleUtility::LocaleUtility( const char* locale ) :
   m_locale( GenerateLocale( locale ))
{
}


LocaleUtility::LocaleUtility( const LocaleUtility& o ) :
   m_locale(o.m_locale)
{
}

const LocaleUtility&
LocaleUtility::operator=( const LocaleUtility& o ) {
   if ( this != &o ) {
      m_locale = o.m_locale;
   }
   return *this;
}

LocaleUtility::~LocaleUtility() {
}

int 
LocaleUtility::strcmp(const LocaleStringRef& strA, 
                      const LocaleStringRef& strB) const
{
   typedef std::collate<char> collate_t;
   const collate_t& collator = use_facet<collate_t>( m_locale );
   return collator.compare( strA.begin(), strA.end(), 
                            strB.begin(), strB.end() );
}

MC2String
LocaleUtility::makeDate( time_t t ) const {
   // The appropriate date format.
   typedef std::time_put<char> ctime_t;
   const ctime_t& timeFacet = std::use_facet< ctime_t >( m_locale );
   std::stringstream s;
   s.imbue( m_locale );
   struct tm tms;
   gmtime_r( &t, &tms );

   timeFacet.put( s, s, s.fill(), &tms, 'x' );

   return MC2String( s.str() );
}

// Unused code 
#if 0
//The code in this ifdef is a first attempt at a strncmp and
//strncasecmp for locales. It will be finished when we need it. 

template<class type>
struct NoOp : public unary_function<type, type>
{
   const type& operator()(const type& arg) 
   {
      return arg;
   }
}

inline int 
LocaleUtility::strncmp(const char* strA, const char* strB, size_t n)
{
   //The facet used for converting between utf8 and unicode.
   typedef codecvt<wchar_t, char, mbstate_t> utf8cvt;

   /** Holds all data needed for the conversion.*/
   template<class CHAR_FUNC = NoOp<utf8cvt::intern_type> >
   class cvt_data {
      /** Reference to the codecvt object*/
      const utf8cvt& cvt;
      /** String that stores the wide character we are looking at. */
      utf8cvt::intern_type wide[2];
      /** Always points to the start of the next utf8 character. */
      const utf8cvt::extern_type* next;
      /** Always points to the end of the utf8 string. */
      const utf8cvt::extern_type* end;
      /** State variable for the utf8converter. */
      utf8cvt::state_type state;
      /** Holds a single mb character for collation*/
      utf8cvt::extern_type mb[12];
      /**
       * This functor will be used to tranform the decoded wide
       * character before comparison.
       */
      CHAR_FUNC transform;
   public:
      cvt_data(const char* utf8, const utf8cvt& cvt) : 
         cvt(cvt), next(utf8), end(utf8 + strlen(utf8)), state()
      {
         wide[0] = wide[1] = 0;
         memset(mb, 0, sizeof(mb));
      }

      utf8cvt::intern_type wideChar() const
      {
         return wide[0];
      }

      enum codecvt_base::result readMb()
      {
         const utf8cvt::extern_type* tmpstart = next;
         utf8cvt::intern_type* next_wide = NULL;
         enum codecvt_base::result res = cvt.in(state, 
                                                tmpstart, end,      next,
                                                wide,     wide + 1, next_wide);
         wide = transform(wide);
         return res;
      }
      
      enum codecvt_base::result convertToMb()
      {
         char* end = NULL;
         const wchar_t* wend = NULL;
         mbstate_t state = mbstate_t();

         enum codecvt_base::result res = cvt.out(state, 
                                                 wide, wide + 1, wend,
                                                 mb,   mb + 7,   end);
         return res;
      }
      const char* Mb() const
      {
         return mb;
      }
   };

   const utf8cvt& cvt = use_facet<utf8cvt>(loc);
   struct cvt_data d1(s1, cvt);
   struct cvt_data d2(s2, cvt);

   size_t first_mb = min( d1.first_mb_n(n), d2.first_mb_n(n) )
   
   for(size_t i = 0; (i < n) && (d1.wideChar() == d2.wideChar()); ++i){
      enum codecvt_base::result res1 = d1.readMb();
      enum codecvt_base::result res2 = d2.readMb();
   }

   d1.convertToMb();
   d2.convertToMb();   

   return use_facet<collate>(loc).compare(d1.Mb(), d1.Mb());
}
#endif


#else

namespace {
   __locale_t GenerateLocale( const char* locale_str )
   {
      __locale_t loc = __newlocale( 1 << LC_ALL, locale_str, NULL );
      if ( loc == NULL ) {
         loc = __newlocale( 1 << LC_ALL, c_backupLocale, NULL );
      }
      if ( loc == NULL ) {
         loc = __newlocale( 1 << LC_ALL, "C", NULL );
      }
      return loc;
   }

   __locale_t DuplicateLocale( const __locale_t fromloc )
   {
      __locale_t loc = __duplocale( fromloc );
      if ( loc == NULL ) {
         loc = GenerateLocale( c_backupLocale );
      }
      return loc;
   }  
   void SwitchLocale( __locale_t& dst, const __locale_t src )
   {
      __locale_t tmp = DuplicateLocale( src );
      if ( tmp != NULL ) {
         __freelocale( dst );
         dst = tmp;
      }  
   }
}

LocaleUtility::LocaleUtility( LangTypes::language_t lang ) :
   m_locale(NULL)
{
   m_locale = GenerateLocale( LangTypes::getUtf8LocaleAsString( lang ));
   MC2_ASSERT( m_locale );
}

LocaleUtility::LocaleUtility( const char* locale ) :
   m_locale( NULL )
{
   m_locale = GenerateLocale( locale );
   MC2_ASSERT( m_locale );
}

LocaleUtility::LocaleUtility( const LocaleUtility& o ) :
   m_locale( NULL )
{
   m_locale = DuplicateLocale( o.m_locale );
   MC2_ASSERT( m_locale );
}

const LocaleUtility&
LocaleUtility::operator=( const LocaleUtility& o ) {
   if ( this != &o ) {
      SwitchLocale( m_locale, o.m_locale );
      MC2_ASSERT( m_locale );
   }
   return *this;
}

LocaleUtility::~LocaleUtility() {
   __freelocale( m_locale );
}

int 
LocaleUtility::strcmp(const LocaleStringRef& strA, 
                      const LocaleStringRef& strB) const
{
   return __strcoll_l( strA.begin(), strB.begin(), m_locale);
}


// Some buggy versions of gcc complain about the use of %c: warning: '%c'
// yields only last 2 digits of year in some locales.
// This happens for %x too.
size_t my_strftime( char *s, size_t max, const char *fmt,  const
                    struct tm *tm ) {
   return strftime( s, max, fmt, tm );
}

MC2String
LocaleUtility::makeDate( time_t t ) const {
   // The appropriate date format.
   const uint32 maxSize = 40;
   char dateStr [ maxSize ];

   struct tm tms;
   gmtime_r( &t, &tms );

   // _DATE_FMT Thu Feb 15 16:24:26 GMT 2007 for almost all
   // ERA_D_FMT empty string...
   // _NL_WD_FMT a % char nothing else
   // _NL_WERA_D_FMT  blank
   // _NL_W_DATE_FMT  a % char nothing else
   char* format = __nl_langinfo_l( D_FMT, m_locale );
   if ( format == NULL ) {
      format = "%x";
   }
   if ( my_strftime( dateStr, maxSize, format, &tms ) <= 0 ) {
      dateStr[ 0 ] = '\0';
   }

   return MC2String( dateStr );
}


#endif

// Methods not CPP_LOCALES depandant

MC2String
LocaleUtility::makeDate( time_t t, LangTypes::language_t lang ) {
   LocaleUtility u( lang );
   return u.makeDate( t );
}

