/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "STLStringUtility.h"
#include "StringUtility.h"

#include <stdlib.h>

MC2String
STLStringUtility::addPathComponent( const MC2String& src_path,
                                    const MC2String& toadd,
                                    const char* dirSep )
{
   MC2String dest_path = src_path;
   if ( dest_path.empty() ||
        dest_path.rfind( dirSep ) != dest_path.size() - 1 ) {

      if (!toadd.empty() && toadd[0] != '/') {
         dest_path += dirSep;
      }
   }
   dest_path += toadd;
   return dest_path;
}

MC2String 
STLStringUtility::basename_no_suffix( const MC2String& str,
                                      char dirSep )
{
   int pos = str.rfind( dirSep );
   if ( pos < 0 ) {
      return MC2String( str );
   } else {
      return MC2String( str, pos + 1 );
   }
}

MC2String
STLStringUtility::basename( const MC2String& name,
                            const MC2String& suffix,
                            char dirsep )
{
   if ( suffix.empty() ) {
      return basename_no_suffix( name, dirsep );
   } else {
      MC2String basename1 ( basename_no_suffix( name, dirsep ) );
      // Now remove the suffix from the basename1.
      MC2String::size_type findPos = basename1.rfind( suffix );
      if ( findPos != MC2String::npos ) {
         return basename1.substr(0, findPos);
      } else {
         return basename1;
      }
   }
}

MC2String 
STLStringUtility::dirname( const MC2String& str, char dirSep )
{
   int pos = str.rfind( dirSep );
   if ( pos < 0 ) {
      return MC2String( "." );
   } else {
      return MC2String( str, 0, pos );
   }
}

MC2String
STLStringUtility::fileExtension( const MC2String& str, bool tolower)
{
   MC2String::size_type findPos = str.rfind( '.' ); // Last .
   
   if ( findPos != MC2String::npos ) {
      MC2String ext = str.substr( findPos + 1 );
      if ( ! tolower ) {
         return ext;
      } else {
         return StringUtility::copyLower( ext );
      }
   }

   // No dot found
   return "";
}


MC2String 
STLStringUtility::breakLine( const MC2String line, uint32 lineWidth,
                             const MC2String breakChars,
                             const MC2String eolStr )
{
   MC2String res;
   if ( res.capacity() < line.size() ) {
      res.reserve( line.size() );
   }

   if ( lineWidth < 4 ) {
      lineWidth = 4;
   }
   
   MC2String::size_type pos = 0;
   uint32 lpos = 0;
   uint32 lastLineBreak = 0;

   while ( lpos < line.size() ) {
      // Find next \n
      pos = line.find( eolStr, lpos );
      if ( line.size() - lpos < lineWidth ) {
         // Just add it
         res.append( line, lpos, (line.size() - lpos) + 1 );
         pos = line.size();
         lastLineBreak = pos;
         lpos = pos;
      } else if ( pos == MC2String::npos || (pos - lpos) > lineWidth ) {
         // Break line
         // Find lineWidth pos and search backwards for blank
         pos = line.find_last_of( breakChars, lpos + lineWidth -1 );
         if ( pos != MC2String::npos && pos > lastLineBreak ) {
            // If found and not before last lastLineBreak break there 
            pos++;
         } else {
            // Break after lineWidth chars, but not after string
            pos = MIN( lpos + lineWidth, line.size() );
         }

         res.append( line, lpos, (pos - lpos) );
         res.append( eolStr );
         lastLineBreak = pos;
         lpos = pos;
      } else { // This line is ok
         res.append( line, lpos, (pos - lpos) + eolStr.size() );
         lastLineBreak = pos;
         lpos = pos + eolStr.size(); // Skipp the eolStr
      }
   }

   return res;
}


MC2String&
STLStringUtility::uint2str( uint32 ui, MC2String& s ) {
   char tmpStr[ 24 ];
   sprintf( tmpStr, "%u", (unsigned int)ui );
   s.append( tmpStr );
   return s;
}

MC2String
STLStringUtility::uint2str( uint32 ui ) {
   MC2String s;
   char tmpStr[ 24 ];
   sprintf( tmpStr, "%u", (unsigned int)ui );
   s.append( tmpStr );
   return s;
}



MC2String&
STLStringUtility::int2str( int32 ui, MC2String& s ) {
   char tmpStr[ 24 ];
   sprintf( tmpStr, "%d", (unsigned int)ui );
   s.append( tmpStr );
   return s;
}


MC2String&
STLStringUtility::int2str( uint64 ui, MC2String& s ) {
   char tmpStr[ 24 ];
   sprintf( tmpStr, "%llu", ui );
   s.append( tmpStr );
   return s;
}

MC2String
STLStringUtility::int2str( int32 i ) {
   MC2String s;
   char tmpStr[ 24 ];
   sprintf( tmpStr, "%d", (int)i );
   s.append( tmpStr );
   return s;
}

MC2String&
STLStringUtility::uint2strAsHex( uint32 ui, MC2String& s ) {
   char tmpStr[ 24 ];
   sprintf( tmpStr, "%x", (unsigned int)ui );
   s.append( tmpStr );
   return s;
}


MC2String&
STLStringUtility::float2Str(  float64 f, MC2String& s ) {
   char tmpStr[ 40 ];
   sprintf( tmpStr, "%f", f );
   s.append( tmpStr );
   return s;
}

uint32 
STLStringUtility::strtol ( const MC2String& str, int base )
{
   return ::strtol( str.c_str(), (char**)NULL, base );
}


uint32 
STLStringUtility::strtoul ( const MC2String& str, int base )
{
   return ::strtoul( str.c_str(), (char**)NULL, base );
}


uint64
STLStringUtility::strtoll ( const MC2String& str, int base )
{
   return ::strtoll( str.c_str(), (char**)NULL, base );
}


uint64
STLStringUtility::strtoull ( const MC2String& str, int base )
{
   return ::strtoull( str.c_str(), (char**)NULL, base );
}

float64
STLStringUtility::strtod ( const MC2String& str )
{
   return ::strtod( str.c_str(), (char**)NULL );
}

bool
STLStringUtility::strtod ( const MC2String& str, float64& val )
{
   char* tmp;
   val = ::strtod( str.c_str(), &tmp );
   
   return ( tmp != str.c_str() && *tmp == '\0' );
}

bool
STLStringUtility::strtoul( const MC2String& str, uint32& val, int base ) {
   char* tmp;
   val = ::strtoul( str.c_str(), &tmp, base );
   
   return ( tmp != str.c_str() && *tmp == '\0' );
}

vector<MC2String>
STLStringUtility::explode( const MC2String& separator, 
                           const MC2String& str )
{
   vector<MC2String> r;
   
   uint32 pos = 0;
   MC2String::size_type findPos = str.find( separator );
   while ( findPos != MC2String::npos ) {
      r.push_back( str.substr( pos, findPos - pos ) );

      pos = findPos + separator.size();
      findPos = str.find( separator, pos ); 
   }
   r.push_back( str.substr( pos ) );

   return r;
}

MC2String 
STLStringUtility::join( const vector<MC2String>& strings, 
                        bool removeEmptyStrings, 
                        const char* separator ) {
   MC2String result = "";
   
   // the first time we append a string we don't add a separator
   bool first = true;
   for ( unsigned i= 0; i < strings.size(); ++i ) {
      if ( removeEmptyStrings && strings[ i ].empty() ) {
         continue;
      }
      
      if ( first ) {
         result += strings[ i ];
         first = false;
      }
      else {
         result += separator + strings[ i ];
      }
   }
   
   return result;   
}


bool
STLStringUtility::replaceString( MC2String& str, 
                                 const MC2String& what,
                                 const MC2String& withThis ) 
{
   size_t pos = str.find( what );
   if ( pos == MC2String::npos ) {
      return false;
   }

   str.replace( pos, what.size(), withThis );

   return true;
}

uint32
STLStringUtility::replaceAllStrings( MC2String& str, 
                                     const MC2String& what,
                                     const MC2String& withThis )
{
   int nbr = 0;
   size_t pos = str.find( what );
   while ( pos != MC2String::npos ) {
      str.replace( pos, what.size(), withThis );
      pos += withThis.size();
      pos = str.find( what, pos );
   }

   return nbr;
}


MC2String
STLStringUtility::splitSeconds( uint32 nbrSec )
{
   // Dangerous, but should be enough for all values of nbrSec
   char tmp[1024];
   return StringUtility::splitSeconds( nbrSec, tmp );
}
