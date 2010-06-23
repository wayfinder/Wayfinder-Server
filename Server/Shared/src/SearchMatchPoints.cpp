/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "SearchMatchPoints.h"
#include "Packet.h"

// - Class SearchMatchPoints ---

void
SearchMatchPoints::setTypes(uint32 searchType,
                            ItemTypes::itemType itemType,
                            uint16 itemSubType)
{
   m_searchType  = searchType;
   m_itemType    = itemType;
   m_itemSubType = itemSubType;
}

int
SearchMatchPoints::getSizeInPacket(bool matchInfoHere) const
{
   // Ugglehack
   Packet* p = new Packet(1000);
   int pos = 0;
   save(p, pos, matchInfoHere);
   delete p;
   mc2dbg << "[SMP]: Size in packet is = " << pos << endl;
   return pos;
}

bool
SearchMatchPoints::save(Packet* p, int& pos, bool matchInfoHere) const
{
   if ( matchInfoHere ) {
      p->incWriteLong(pos, m_searchType);
      p->incWriteShort(pos, m_itemType);
      p->incWriteShort(pos, m_itemSubType);
   }

   // Information not in the matches
   p->incWriteShort(pos, m_houseNbrDiff);
   p->incWriteShort(pos, m_elemPoints);
   p->incWriteShort(pos, m_editDistance);
   // Bytes
   p->incWriteByte(pos, m_stringDiff);
   p->incWriteByte(pos, m_positionInString);
   p->incWriteByte(pos, m_matching);
   p->incWriteByte(pos, m_matchedStringIs);

   // Some boolean values.
   int bitPos = 0;
   p->writeBit(pos, bitPos++, m_correctLocation);
   p->writeBit(pos, bitPos++, m_ssiCorrectLocation);
   p->writeBit(pos, bitPos++, m_matchesEndOfWord);
   p->writeBit(pos, bitPos++, m_matchesBeginningOfWord);
   p->writeBit(pos, bitPos++, m_allWordsCorrectOrder);
   ++pos;
   
   return true;
}

bool
SearchMatchPoints::load(const Packet* p, int& pos, bool matchInfoHere)
{
   reset();
   if ( matchInfoHere ) {
      // The distance may not be necessary, since
      // it cannot be calculated in SearchModule
      m_searchType = p->incReadLong(pos);
      m_itemType   = ItemTypes::itemType(p->incReadShort(pos));
      m_itemSubType = p->incReadShort(pos);
   }

   // Information not in the matches
   // The shorts
   m_houseNbrDiff = p->incReadShort(pos);
   m_elemPoints   = p->incReadShort(pos);
   m_editDistance = p->incReadShort(pos);
   
   // The bytes
   m_stringDiff       = p->incReadByte(pos);
   m_positionInString = p->incReadByte(pos);
   m_matching         = SearchTypes::StringMatching(p->incReadByte(pos) );
   m_matchedStringIs  = matchedString_t( p->incReadByte(pos) );
                                        
   
   // The bits
   int bitPos = 0;  
   m_correctLocation        = p->readBit(pos, bitPos++);
   m_ssiCorrectLocation     = p->readBit(pos, bitPos++);
   m_matchesEndOfWord       = p->readBit(pos, bitPos++);
   m_matchesBeginningOfWord = p->readBit(pos, bitPos++);
   m_allWordsCorrectOrder   = p->readBit(pos, bitPos++);
   ++pos;

   return true;
}

uint16
SearchMatchPoints::getPoints( bool debugp ) const
{
   uint16 retVal = 0;

   // We will try to add the extra points for
   // type. ( From SearchMapGenerator )
   if ( m_itemType == ItemTypes::pointOfInterestItem ) {
      ItemTypes::pointOfInterest_t p =
         ItemTypes::pointOfInterest_t(m_itemSubType);
      uint16 selectionCount = 0;
      if ((p == ItemTypes::company) ||
          (p == ItemTypes::unknownType))
      {
         if ( debugp ) {
            mc2dbg << "POI::comp" << endl;
         }
         // no change. selection count = 0.
      } else if ((p == ItemTypes::cityCentre)) {
         if ( debugp ) {
            mc2dbg << "POI::cityc" << endl;
         }
         selectionCount += 90;
      } else if ((p == ItemTypes::airport) ||
                 (p == ItemTypes::busStation) ||
                 (p == ItemTypes::commuterRailStation) ||
                 (p == ItemTypes::railwayStation))
      {
         if ( debugp ) {
            mc2dbg << "POI::import" << endl;
         }
         selectionCount += 85; // TODO: move to SearchTypes.
      } else {
         if ( debugp ) {
            mc2dbg << "POI::other type" << endl;
         }
         selectionCount += 50; // TODO: move to SearchTypes.
      }

      retVal += selectionCount;
   }

   if ( m_matching == SearchTypes::SoundexMatch) {      
      if ( m_editDistance != MAX_UINT16 ) {
         // NOTE !!!!!!!!!!!!!!!!!
         retVal = 0;
         retVal += 27 - MIN(27, m_editDistance);
         // RETURNING
         return retVal;         
      }
   } else {
      if ( m_matchedStringIs == close ) {
         if ( debugp ) {
            mc2dbg << "matching close" << endl;
         }
         retVal += SearchTypes::matchedStringIsClosePoints;
      } else if ( m_matchedStringIs == exact ) {
         if ( debugp ) {
            mc2dbg << "matching exact" << endl;
         }
         retVal += SearchTypes::matchedStringIsExactPoints;
      } else if ( m_matchedStringIs == full_exact ||
                  m_matchedStringIs == full_close) {
         if ( debugp ) {
            mc2dbg << "matching full" << endl;
         }
         retVal += SearchTypes::matchedStringIsExactPoints * 4;
      }
   }

   if ( m_houseNbrDiff == 0 ) {
      if ( debugp ) {
         mc2dbg << "m_houseNbrDiff == 0" << endl;
      }
      retVal += 10;
      // Only use the ssicorrectlocation
      // if the housenumbers are correct too.
      if ( m_ssiCorrectLocation ) {
         if ( debugp ) {
            mc2dbg << "m_ssiCorrectLocation" << endl;
         }
         retVal += 5;
      }
   }

   if ( m_matching == SearchTypes::AllWordsMatch ) {
      if ( debugp ) {
         mc2dbg << "m_matching == SearchTypes::AllWordsMatch" << endl;
      }
      retVal += 50;
      if ( m_allWordsCorrectOrder ) {
         retVal += 10;
      }
   }
   
   if ( m_correctLocation  || 
        (m_itemType    == ItemTypes::pointOfInterestItem &&
         m_itemSubType == ItemTypes::airport) ) 
      // Airports have no location at all, but if hit then they are "near"
   {
      if ( debugp ) {
         mc2dbg << "m_correctLocation" << endl;
      }
      retVal += 50;
   }

   const bool wholeWordMatched =
      (m_matchesEndOfWord && m_matchesBeginningOfWord);

   if ( wholeWordMatched ) {
      if ( debugp ) {
         mc2dbg << "wholeWordMatched" << endl;
      }
      retVal += 51;
   } else if ( m_positionInString == 0 ) {
      if ( debugp ) {
         mc2dbg << "m_positionInString == 0" << endl;
      }
      retVal += 50;
   } else if ( m_matchesBeginningOfWord ) {
      if ( debugp ) {
         mc2dbg << "matchesBeginningOfWord" << endl;
      }
      retVal += 49;
   }
   
   if ( m_positionInString == 0 && wholeWordMatched ) {
      if ( debugp ) {
         mc2dbg << "m_positionInString == 0 && wholeWordMatched" << endl;
      }
      retVal += 5;
   } 
      
   // Try it without the element points
   //retVal += m_elemPoints;
   return retVal;
}


// - End of SearchMatchPoints
