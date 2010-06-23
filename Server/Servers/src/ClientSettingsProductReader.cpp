/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ClientSettingsProductReader.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include <regex.h>


ClientSettingsProductReader::
ClientSettingsProductReader( const MC2String& tag ):
   LineParser( tag ) {
}

bool
ClientSettingsProductReader::parseLine( const Line& lineIn ) {
   const MC2String& line = lineIn.getLine();
   const uint32 labelSize = getTag().size();
   MC2_ASSERT( isTagLine( line ) );
   bool ok = true;

   // Find product name
   MC2String::size_type endProdPos = line.find( ";", labelSize );
   
   MC2String productName;
   if ( endProdPos != MC2String::npos ) {
      productName = line.substr( labelSize, endProdPos - labelSize );
   }

   vector<MC2String> clientStrings = STLStringUtility::explode( 
      ",", line.substr( endProdPos + 1, line.find( "\n" ) - endProdPos -1 ) );

   if ( ! clientStrings.empty() ) {
      for ( uint32 i = 0 ; i < clientStrings.size() ; ++i ) {
         // Check pattern!
         MC2String errorStr;
         MC2String patt( StringUtility::trimStartEnd( clientStrings[ i ] ) );
         if ( !StringUtility::regexpCheck( patt, errorStr ) ) {
            mc2log << error << "ClientSettingsProductReader bad pattern: "
                   << MC2CITE( clientStrings[ i ] ) << " error: "
                   << errorStr << endl;
            ok = false;
         } else {
            m_productPatterns.
               push_back( STLUtility::make_vpair( patt, productName ) );
         }
      }
   } else {
      mc2log << error << "ClientSettingsProductReader no entries on Product "
             << "line: " << lineIn << endl;
      ok = false;
   }

   return ok;
}


const char*
ClientSettingsProductReader::
getProduct( const MC2String& client_type ) const {
   for ( productPattersCont::const_iterator it = m_productPatterns.begin();
         it != m_productPatterns.end(); ++it ) {
      if ( StringUtility::regexp( it->key, client_type )  ) {
         return it->value.c_str();
      }
   }

   return "";
}
