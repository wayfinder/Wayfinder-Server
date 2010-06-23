/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PersistentSearchIDs.h"
#include "SearchParserHandler.h"
#include "SearchMatch.h"
#include "MC2Itoa.h"

namespace PersistentSearchIDs {

bool
isPersistentID( const MC2String& itemIDStr ) {
   return itemIDStr[ 0 ] == 'P';
}


MC2String 
makeID( const SearchMatch& match,
        const SearchParserHandler& searchHandler ) {
   // 'P' 6 octets lat 6 octets lon headingID ':' name 
   // And a maximum of 64 octets in total. Name is ended at last blank
   // char or at last (utf-8) character.

   // SearchMatch "Persistent" ID identifier
   MC2String id( "P" );

   // Lat
   id.append( MC2Itoa::toStr( match.getCoords().lat, 64, 6 ) );

   // Lon
   id.append( MC2Itoa::toStr( match.getCoords().lon, 64, 6 ) );

   // Heading
   uint32 headingID = searchHandler.getHeadingForMatch( &match );
   id.append( MC2Itoa::toStr( headingID, 64 ) );
   id.append( ":" );

   // Name
   MC2String name( match.getName() );
   // Companies may have street address in it's name
   if ( (match.getType() & SEARCH_COMPANIES) != 0 ) {
      const VanillaCompanyMatch* company = 
         static_cast<const VanillaCompanyMatch*> ( &match );
      if ( ! company->getCleanCompanyName().empty() ) {
         name = company->getCleanCompanyName();
      }
   }
   // Check and limit id size
   const uint32 maxSize = 4096;
   if ( id.size() + name.size() > maxSize ) {
      // Find last whitespace (and separators ,') in name until size <= maxSize
      // TODO: Make a function of this so it can be reused (smartCutString)
      const MC2String separators( " \t,.;" );
      size_t sepPos = name.find_last_of( separators );
      while ( sepPos != MC2String::npos && sepPos > 0 &&
              id.size() + sepPos > maxSize ) 
      {
         sepPos = name.find_last_of( separators, sepPos - 1 );
      }
      // If still not <= maxSize (like when no whitespace) cut at last valid
      // character (utf-8)
      if ( sepPos == MC2String::npos || id.size() + sepPos > maxSize ) {
         mc2TextIterator nameIt( name );
         sepPos = 0;
         while ( id.size() + sepPos + nameIt.nbrBytesForCurrentChar() <=
                 maxSize ) {
            sepPos += nameIt.nbrBytesForCurrentChar();
            ++nameIt;
         }
      }
      // Cut at separator, do not include separator
      name = name.substr( 0, sepPos );
   }
   id.append( name );


   // Other formats for comparison
#if 0
   // Type 1
   MC2String id1( "P" );
   id1.append( MC2Itoa::toStr( match.getCoords().lat, 64, 6 ) );
   id1.append( MC2Itoa::toStr( match.getCoords().lon, 64, 6 ) );
   id1.append( MC2Itoa::toStr( headingID, 64 ) );
   id1.append( ":" );
   // id1.append( match.getName() );

   // Type 2
   MC2String id2( "P" );
   id2.append( MC2Itoa::toStr( match.getCoords().lat, 64, 6 ) );
   id2.append( MC2Itoa::toStr( match.getCoords().lon, 64, 6 ) );
   id2.append( MC2Itoa::toStr( headingID, 64, 6 ) );
   // id2.append( match.getName() );

   // Type 3
   MC2String id3( "P" );
   id3.append( MC2Itoa::toStr( match.getCoords().lat, 64 ) );
   id3.append( ":" );
   id3.append( MC2Itoa::toStr( match.getCoords().lon, 64 ) );
   id3.append( ":" );
   id3.append( MC2Itoa::toStr( headingID, 64 ) );
   id3.append( ":" );
   // id3.append( match.getName() );

   mc2dbg << "id1: " << id1 << endl;
   mc2dbg << "id2: " << id2 << endl;   
   mc2dbg << "id3: " << id3 << endl;   
   mc2dbg << id1.size() << " vs. " << id2.size() << " vs. " << id3.size() 
          << endl;   
#endif

   return id;
}

bool
getMatchData( const MC2String& itemIDStr,
              const SearchParserHandler& searchHandler,
              MC2Coordinate& coord,
              MC2String& name,
              uint32& headingID ) {
   if ( ! isPersistentID( itemIDStr ) ) {
      mc2log << warn << "[PSID]:getMatchData not persistent id " 
             << MC2CITE( itemIDStr ) << endl;
      return false;
   }
   if ( itemIDStr.size() < 15 ) {
      mc2log << warn << "[PSID]:getMatchData too short persistent id " 
             << MC2CITE( itemIDStr ) << endl;
      return false;
   }

   bool ok = true;
   // Lat
   coord.lat = MC2Itoa::fromStr( itemIDStr.substr( 1, 6 ), 64 );
   // Lon
   coord.lon = MC2Itoa::fromStr( itemIDStr.substr( 7, 6 ), 64 );
   size_t findPos = itemIDStr.find( ':', 13 );
   if ( findPos != MC2String::npos ) {
      // Heading
      headingID = MC2Itoa::fromStr( itemIDStr.substr( 13, findPos - 13 ), 64 );
      // Name
      name = itemIDStr.c_str() + findPos + 1;
   } else {
      mc2log << warn << "[PSID]:getMatchData format error no ':' after "
             << "headingID " << MC2CITE( itemIDStr ) << endl;
      ok = false;
   }

   if ( ok ) {
      // All is set
#if 0
      // Search type from heading
      uint32 searchTypeMask = searchHandler.getSearchTypeForHeading( 
         headingID );
      mc2dbg << "Coord " << coord << " heading " << headingID << " Name "
             << name << " searchTypeMask " << MC2HEX(searchTypeMask) << endl;
      result.reset( SearchMatch::createMatch( searchTypeMask ) );
      if ( result.get() ) {
         result->setCoords( coord );
         result->setName( name );
      } else {
         mc2log << warn << "[PSID]:getMatchData failed to create SearchMatch "
                << "from " << MC2CITE( itemIDStr ) << endl;
         ok = false;
      }
#endif
   }

   return ok;
}

} // End namespace PersistentSearchIDs
