/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ClientSettingsWFIDReader.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include <regex.h>

ClientSettingsWFIDReader::ClientSettingsWFIDReader( const MC2String& tag )
      : LineParser( tag )
{
}

bool
ClientSettingsWFIDReader::parseLine( const Line& lineIn ) {
   const MC2String& line = lineIn.getLine();
   const uint32 labelSize = getTag().size();
   MC2_ASSERT( isTagLine( line ) );
   bool ok = true;

   vector<MC2String> clientStrings = STLStringUtility::explode( 
      ",", line.substr( labelSize, line.find( "\n" ) - labelSize ) );

   if ( clientStrings.size() > 0 ) {
      for ( uint32 i = 0 ; i < clientStrings.size() && ok ; ++i ) {
         // Check pattern!
         MC2String errorStr;
         MC2String patt( StringUtility::trimStartEnd( clientStrings[ i ] ) );
         if ( !StringUtility::regexpCheck( patt, errorStr ) ) {
            mc2log << error << "ClientSettingsWFIDReader " << getTag() 
                   << " bad pattern: " << MC2CITE( clientStrings[ i ] )
                   << " error: " << errorStr << endl;
            ok = false;
         } else {
            m_wfidPatterns.insert( patt );
         }
      }
   } else {
      mc2log << error << "ClientSettingsWFIDReader no entries on " << m_tag 
             << " line: " << line << endl;
      ok = false;
   }

   return ok;
}


bool
ClientSettingsWFIDReader::isSet( const MC2String& clientType ) {
   bool found = false;

   for ( wfidPattersCont::const_iterator it = m_wfidPatterns.begin() ;
         it != m_wfidPatterns.end() ; ++it ) {
      if ( StringUtility::regexp( *it, clientType )  ) {
         found = true;
         break;
      }
   }

   return found;
}
