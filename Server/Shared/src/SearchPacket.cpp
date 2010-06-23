/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SearchPacket.h"

VanillaSearchRequestPacket::VanillaSearchRequestPacket() 
      : OldSearchRequestPacket( PACKETTYPE_VANILLASEARCHREQUEST,
                             0,
                             0 ) {
}

UserSearchRequestPacket::UserSearchRequestPacket()
    : OldSearchRequestPacket( PACKETTYPE_USERSEARCHREQUEST,
                           0,
                           0 ) {}

UserSearchRequestPacket::UserSearchRequestPacket( uint16 packetID,
                                                  uint16 reqID )
      : OldSearchRequestPacket( PACKETTYPE_USERSEARCHREQUEST,
                              packetID,
                              reqID ) {}


VanillaSearchRequestPacket::VanillaSearchRequestPacket( uint16 packetID,
                                                        uint16 reqID )
      : OldSearchRequestPacket( Packet::PACKETTYPE_VANILLASEARCHREQUEST,
                             packetID,
                             reqID ) {
}


void VanillaSearchRequestPacket::encodeRequest(
   uint32  numMapID,
   const uint32* mapID,
   const char* zipCode,
   uint32 nbrLocations,
   const char** locations,
   uint32 locationType,
   uint32 nbrCategories,
   const uint32* categories,
   uint32 nbrMasks,
   const uint32* masks,
   const uint32* maskItemIDs,
   const char** maskNames,
   uint8 dbMask,
   const char* searchString,
   uint8 nbrHits,
   SearchTypes::StringMatching matchType,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   uint16 categoryType,
   uint16 requestedLanguage,
   uint32 regionsInMatches,
   StringTable::countryCode topRegion )
{
   if ( numMapID == 1 ) {
      RequestPacket::setMapID( mapID[ 0 ] );
   }
   DEBUG8(
      for ( uint32 w = 0 ; w < numMapID ; w++ ) {
         mc2dbg << "VanillaEncode MapID = " << mapID[w] << endl;         
      }
      );
   int position = writeHeader( sortingType, numMapID, mapID, zipCode, 
                               nbrLocations,
                               locations, locationType,
                               nbrCategories, categories,
                               nbrMasks, masks, maskItemIDs,
                               maskNames,
                               dbMask, regionsInMatches, topRegion,
                               vector<MC2BoundingBox>());
   incWriteString( position, searchString);
   incWriteByte(   position, nbrHits );
   incWriteByte(   position, matchType );
   incWriteByte(   position, stringPart );
   incWriteByte(   position, sortingType );
   incWriteShort(  position, categoryType );
   incWriteShort(  position, requestedLanguage );
   setLength(position); 
}


void UserSearchRequestPacket::encodeRequest(
   uint32  numMapID,
   const uint32* mapID,
   const char* zipCode,
   uint32 nbrLocations,
   const const_char_p*  locations,
   uint32 locationType,
   uint32 nbrCategories,
   const uint32* categories,
   uint32 nbrMasks,
   const uint32* masks,
   const uint32* maskItemIDs,
   const const_char_p* maskNames,
   uint8 dbMask,
   const char* searchString,
   uint8 nbrHits,
   SearchTypes::StringMatching matchType,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   uint16 categoryType,
   LangTypes::language_t requestedLanguage,
   uint8 nbrSortedHits,
   uint16 editDistanceCutoff,
   uint32 regionsInMatches,
   const Categories& realCategories,
   StringTable::countryCode topRegion,
   const vector<MC2BoundingBox>& bboxes)
{
   if ( numMapID == 1 ) {
      RequestPacket::setMapID( mapID[ 0 ] );
   }
   DEBUG8(
      for ( uint32 w = 0 ; w < numMapID ; w++ ) {
         mc2dbg << "UserEncode MapID = " << mapID[w] << endl;         
      }
      );
      
   int position = writeHeader( sortingType, numMapID, mapID, zipCode, 
                               nbrLocations, (const char**)locations,
                               locationType,
                               nbrCategories, categories,
                               nbrMasks, masks, maskItemIDs,
                               (const char**)maskNames,
                               dbMask, regionsInMatches, topRegion,
                               bboxes);

   // Double the size if there isn't room for 50 more bytes.
   setLength(position);
   if ( updateSize( 50, getBufSize()) ) {
      mc2dbg << "[SReqP]: Resized packet to " << getBufSize()
             << " bytes" << endl;
   }
   
   incWriteString( position, searchString);
   incWriteByte(   position, nbrHits );
   incWriteByte(   position, matchType );
   incWriteByte(   position, stringPart );
   incWriteByte(   position, sortingType );
   incWriteShort(  position, categoryType );
   incWriteShort(  position, requestedLanguage );
   incWriteShort(  position, editDistanceCutoff);
   incWriteByte(   position, nbrSortedHits );
   // write categories
   incWriteLong( position, realCategories.size() );
   Categories::const_iterator it = realCategories.begin();
   Categories::const_iterator itEnd = realCategories.end();
   for ( ; it != itEnd; ++it ) {
      incWriteShort( position, *it );
   }

   setLength(position); 
}


void
VanillaSearchRequestPacket::decodeRequest(
   char*&   zipCode,
   uint32&  nbrLocations,
   char**&  locations,
   uint32&  locationType,
   uint32&  nbrCategories,
   uint32*& categories,
   uint32&  nbrMasks,
   uint32*& masks,
   uint32*& maskItemIDs,
   char**&  maskNames,
   uint8&   dbMask,
   char*&   searchString,
   uint8&   nbrHits,
   SearchTypes::StringMatching& matchType,
   SearchTypes::StringPart&     stringPart,
   SearchTypes::SearchSorting&  sortingType,
   uint16&           categoryType,
   uint16& requestedLanguage,
   uint32& regionsInMatches,
   StringTable::countryCode& topRegion ) const
{
   vector<MC2BoundingBox> bboxes;
   int position = readHeader( sortingType, zipCode, nbrLocations, 
                              locations, locationType,
                              nbrCategories, categories,
                              nbrMasks, masks, 
                              maskItemIDs, maskNames,
                              dbMask, regionsInMatches, topRegion,
                              bboxes);
   
   incReadString( position, searchString );
   nbrHits = incReadByte( position );
   matchType   = static_cast<SearchTypes::StringMatching>
      ( incReadByte( position ) );
   stringPart  = static_cast<SearchTypes::StringPart>
      ( incReadByte( position ) );
   sortingType = static_cast<SearchTypes::SearchSorting>
      ( incReadByte( position ) );
   categoryType = incReadShort( position );
   requestedLanguage = incReadShort( position );
   
   mc2dbg8 << "sorting" << int(sortingType) << endl;
   mc2dbg8 << "categoryType" << (int)categoryType << endl;
}

void UserSearchRequestPacket::decodeRequest(
   char*& zipCode,
   uint32& nbrLocations,
   char**& locations,
   uint32& locationType,
   uint32& nbrCategories,
   uint32*& categories,
   uint32& nbrMasks,
   uint32*& masks,
   uint32*& maskItemIDs,
   char**& maskNames,
   uint8& dbMask,
   char*& searchString,
   uint8& nbrHits,
   SearchTypes::StringMatching& matchType,
   SearchTypes::StringPart&     stringPart,
   SearchTypes::SearchSorting&  sortingType,
   uint16& categoryType,
   uint16& requestedLanguage,
   uint8 &nbrSortedHits,
   uint16& editDistanceCutoff,
   uint32& regionsInMatches,
   Categories& realCategories,
   StringTable::countryCode& topRegion,
   vector<MC2BoundingBox>& bboxes) const
{
   int position = readHeader( sortingType, zipCode, nbrLocations, 
                              locations, locationType,
                              nbrCategories, categories,
                              nbrMasks, masks,
                              maskItemIDs, maskNames,
                              dbMask, regionsInMatches, topRegion,
                              bboxes);

   incReadString( position, searchString );

   nbrHits = incReadByte( position );
   matchType   = static_cast<SearchTypes::StringMatching>
      ( incReadByte( position ) );
   stringPart  = static_cast<SearchTypes::StringPart>
      ( incReadByte( position ) );
   sortingType = static_cast<SearchTypes::SearchSorting>
      ( incReadByte( position ) );
   categoryType = incReadShort( position );
   requestedLanguage = incReadShort( position );
   editDistanceCutoff = incReadShort(position);
   nbrSortedHits = incReadByte( position );
   
   uint32 nbrRealCategories = incReadLong( position );
   for ( uint32 i = 0; i < nbrRealCategories; ++i ) {
      realCategories.insert( incReadShort( position ) );
   }

   mc2dbg8 << "sorting" << int(sortingType) << endl;
   mc2dbg8 << "categoryType" << (int)categoryType << endl;
}


const char* 
UserSearchRequestPacket::getSearchString() const
{
   int position = getHeaderEndPosition();
   char* searchString;
   incReadString( position, searchString );
   return (searchString);
}

SearchSelectionReportPacket::SearchSelectionReportPacket()
      : RequestPacket(Packet::PACKETTYPE_SEARCHSELECTIONREPORT)
{
   setPriority( 0 );
}

uint32
SearchSelectionReportPacket::getNbrSelections()
{
   return readLong( REQUEST_HEADER_SIZE );
}

void
SearchSelectionReportPacket::addSelection(uint32 mapID,
                                          uint32 itemID)
{
   uint32 nbrSelections = getNbrSelections();
   int startPos = REQUEST_HEADER_SIZE+sizeof(uint32)+
      (sizeof(mapID)+sizeof(itemID))*nbrSelections;
   incWriteLong(startPos, mapID);
   quickIncWriteLong(startPos, itemID);
   setLength( startPos );
   writeLong( REQUEST_HEADER_SIZE, nbrSelections+1 );
}

void
SearchSelectionReportPacket::getSelection(uint32 index,
                                          uint32 &mapID,
                                          uint32 &itemID)
{
   if (index < getNbrSelections()) {
      int startPos = REQUEST_HEADER_SIZE+sizeof(uint32)+
         (sizeof(mapID)+sizeof(itemID))*index;
      mapID  = incReadLong(startPos);
      itemID = quickIncReadLong(startPos);
   } else {
      mc2log << error
             << "Index out of range in SearchSelectionReportPacket::"
             << "getSelection." << endl;
      mapID  = MAX_UINT32;
      itemID = MAX_UINT32;
   }
}

//////////////////////////////////////////////////////////////////////////
////////////////   ExpandCategory2SearchRequestPacket

ExpandCategory2SearchRequestPacket::
ExpandCategory2SearchRequestPacket()
    : OldSearchRequestPacket( PACKETTYPE_EXPANDCATEGORY2SEARCHREQUEST,
                           0,
                           0 ) {}

ExpandCategory2SearchRequestPacket::
ExpandCategory2SearchRequestPacket( uint16 packetID,
                                                  uint16 reqID )
      : OldSearchRequestPacket( PACKETTYPE_EXPANDCATEGORY2SEARCHREQUEST,
                              packetID,
                              reqID ) {}

void
ExpandCategory2SearchRequestPacket::
encodeRequest( uint32  numMapID,
               const uint32* mapID,
               const char* zipCode,
               uint32 nbrLocations,
               const char** locations,
               uint32 locationType,
               uint32 nbrCategories,
               const uint32* categories,
               uint32 nbrMasks,
               const uint32* masks,
               const uint32* maskItemIDs,
               const char** maskNames,
               uint8 dbMask,
               const char* searchString,
               uint8 nbrHits,
               SearchTypes::StringMatching matchType,
               SearchTypes::StringPart stringPart,
               SearchTypes::SearchSorting sortingType,
               uint16 categoryType,
               uint16 requestedLanguage,
               uint8 k,
               uint32 regionsInMatches,
               StringTable::countryCode topRegion )
{
   if ( numMapID == 1 ) {
      RequestPacket::setMapID( mapID[ 0 ] );
   }
   DEBUG8(
      for ( uint32 w = 0 ; w < numMapID ; w++ ) {
         mc2dbg << "UserEncode MapID = " << mapID[w] << endl;         
      }
      );
      
   int position = writeHeader( sortingType, numMapID, mapID, zipCode, 
                               nbrLocations, locations, locationType,
                               nbrCategories, categories,
                               nbrMasks, masks, maskItemIDs, maskNames,
                               dbMask, regionsInMatches, topRegion );
   incWriteString( position, searchString);
   incWriteByte(   position, nbrHits );
   incWriteByte(   position, matchType );
   incWriteByte(   position, stringPart );
   incWriteByte(   position, sortingType );
   incWriteShort(  position, categoryType );
   incWriteShort(  position, requestedLanguage );
   incWriteByte(   position, k );
   setLength(position); 
}

void
ExpandCategory2SearchRequestPacket::
decodeRequest( char*& zipCode,
               uint32& nbrLocations,
               char**& locations,
               uint32& locationType,
               uint32& nbrCategories,
               uint32*& categories,
               uint32& nbrMasks,
               uint32*& masks,
               uint32*& maskItemIDs,
               char**& maskNames,
               uint8& dbMask,
               char*& searchString,
               uint8& nbrHits,
               SearchTypes::StringMatching& matchType,
               SearchTypes::StringPart&     stringPart,
               SearchTypes::SearchSorting&  sortingType,
               uint16& categoryType,
               uint16& requestedLanguage,
               uint8 &nbrSortedHits,
               uint32& regionsInMatches,
               StringTable::countryCode& topRegion ) const
{
   vector<MC2BoundingBox> bboxes;
   int position = readHeader( sortingType, zipCode, nbrLocations, 
                              locations, locationType,
                              nbrCategories, categories,
                              nbrMasks, masks,
                              maskItemIDs, maskNames, dbMask, 
                              regionsInMatches, topRegion,bboxes );
   
   incReadString( position, searchString );
   nbrHits = incReadByte( position );
   matchType   = static_cast<SearchTypes::StringMatching>
      ( incReadByte( position ) );
   stringPart  = static_cast<SearchTypes::StringPart>
      ( incReadByte( position ) );
   sortingType = static_cast<SearchTypes::SearchSorting>
      ( incReadByte( position ) );
   categoryType = incReadShort( position );
   requestedLanguage = incReadShort( position );
   nbrSortedHits = incReadByte( position );
   
   mc2dbg8 << "sorting" << int(sortingType) << endl;
   mc2dbg8 << "categoryType" << (int)categoryType << endl;
}
