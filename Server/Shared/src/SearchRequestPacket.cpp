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

#include "SearchRequestPacket.h"
#include "SearchRequestParameters.h"
#include "SearchTypes.h"
#include "UserRightsMapInfo.h"

#include "ItemIDTree.h"

int
SearchRequestPacket::
calcPacketSize(const char* searchString,
               const SearchRequestParameters& params,
               const vector<IDPair_t>& allowedRegions,
               const vector<MC2BoundingBox>& alllowedBBoxes,
               const UserRightsMapInfo& rights )
{
   return PARAMETERS_POS +
      strlen(searchString) + 1 +
      params.getSizeInPacket() +
      allowedRegions.size() * 8 + alllowedBBoxes.size() * 16 +
      rights.getSizeInPacket() +
      4 + // Nbr regions
      4 + // Nbr bboxes.
      params.getMapRights().getSizeInPacket() + 
      4 + // inverted rights
      params.getSizeInPacket();
}


SearchRequestPacket::
SearchRequestPacket(uint32 mapID,
                    uint32 packetID,
                    uint32 reqID,               
                    search_t searchType,
                    const char* searchString,
                    const SearchRequestParameters& params,
                    const vector<IDPair_t>& allowedRegions,
                    const vector<MC2BoundingBox>& allowedBBoxes,
                    const MC2Coordinate& sortOrigin,
                    const UserRightsMapInfo& rights )
      : RequestPacket( calcPacketSize(searchString,
                                      params, allowedRegions, allowedBBoxes,
                                      rights ),
                       
                       DEFAULT_PACKET_PRIO,
                       Packet::PACKETTYPE_SEARCHREQUEST,
                       packetID, // packet id
                       reqID, // request id
                       mapID)
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong(pos, searchType);
   incWriteLong(pos, sortOrigin.lat);
   incWriteLong(pos, sortOrigin.lon);
   params.save(this, pos);
   incWriteString(pos, searchString);
   incWriteLong( pos, params.invertRights() );
   params.getMapRights().save( this, pos );

   {
      // Save the length so it can be written later with the corrent length
      int oldPos = pos;
      incWriteLong( pos, 0 );
      uint32 nbrAdded = 0;
      for( vector<IDPair_t>::const_iterator it = allowedRegions.begin();
           it != allowedRegions.end();
           ++it ) {
         // Only write the ones that go to the correct map.
         if ( it->getMapID() == mapID ) {
            ++nbrAdded;
            incWriteLong(pos, it->getMapID());
            incWriteLong(pos, it->getItemID());
         }
      }
      incWriteLong( oldPos, nbrAdded );
   }
   incWriteLong(pos, allowedBBoxes.size());
   for( vector<MC2BoundingBox>::const_iterator it = allowedBBoxes.begin();
        it != allowedBBoxes.end();
        ++it ) {
      incWriteBBox(pos, *it);
   }
   // And the rights
   rights.save( this, pos );
   // Don't forget to set the position.
   setLength(pos);
}

SearchRequestPacket::search_t
SearchRequestPacket::getSearchType() const
{
   return search_t(readLong(SEARCH_TYPE_POS));
}

MC2Coordinate
SearchRequestPacket::getSortOrigin() const
{
   MC2Coordinate sortOrigin;
   int pos = SORT_ORIGIN_POS;
   incReadLong(pos, sortOrigin.lat);
   incReadLong(pos, sortOrigin.lon);
   return sortOrigin;
}

void
SearchRequestPacket::
getParamsAndRegions(SearchRequestParameters& params,
                    char*& searchString,
                    vector<IDPair_t>& allowedRegions,
                    vector<MC2BoundingBox>& allowedBBoxes,
                    UserRightsMapInfo& rights ) const
{   
   int pos = PARAMETERS_POS;
   params.load(this, pos);
   incReadString(pos, searchString);
   bool invertedRights;
   incReadLong( pos, invertedRights );
   params.setInvertRights( invertedRights );
   MapRights mapRights;
   mapRights.load( this, pos );
   params.setMapRights( mapRights );

   int nbrAllowedRegions = incReadLong(pos);
   allowedRegions.reserve(nbrAllowedRegions);
   for( int i = 0; i < nbrAllowedRegions; ++i ) {
      IDPair_t id;
      id.first = incReadLong(pos);
      id.second = incReadLong(pos);
      allowedRegions.push_back(id);
   }
   int nbrAllowedBBoxes = incReadLong(pos);
   allowedBBoxes.reserve(nbrAllowedBBoxes);
   for( int i = 0; i < nbrAllowedBBoxes; ++i ) {
      MC2BoundingBox bbox;
      incReadBBox(pos, bbox);
      allowedBBoxes.push_back(bbox);
   }
   if ( pos >= (int)getLength() ) {
      mc2dbg << "[SRP]: load - no rights - assuming allMask" << endl;
      UserRightsMapInfo allRight( getMapID(), ~MapRights() );
      rights.swap( allRight );
      return;
   }
   // Also read the rights.
   rights.load( this, pos );
}

OldSearchRequestPacket::OldSearchRequestPacket()
      : RequestPacket( MAX_PACKET_SIZE,  // size 
                       0,
                       0,
                       0,
                       0,
                       MAX_UINT32 )
{
}

OldSearchRequestPacket::OldSearchRequestPacket( uint16 subType,
                                          uint16 packetID,
                                          uint16 reqID )
      : RequestPacket( MAX_PACKET_SIZE,  // size 
                       SEARCH_REQUEST_PRIO,
                       subType,
                       packetID,
                       reqID,
                       MAX_UINT32 )
{
}

OldSearchRequestPacket::OldSearchRequestPacket( uint32 bufLength,
                                          byte prio,
                                          uint16 subType,
                                          uint16 packetId,
                                          uint16 requestID,
                                          uint32 mapID )
      : RequestPacket( bufLength, prio, subType,
                       packetId, requestID, mapID )
{
}

int
OldSearchRequestPacket::writeHeader(SearchTypes::SearchSorting sortingType,
                                 uint32 numMapID,
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
                                 uint32 regionsInMatches,
                                 StringTable::countryCode topRegion,
                                 const vector<MC2BoundingBox>& bboxes)
{
   int position = REQUEST_HEADER_SIZE;
   incWriteLong(position, sortingType);
   incWriteLong(position, numMapID);
   uint32 i = 0;
   for( i = 0; i < numMapID; i++){
      // Double the size if there isn't room for 50 more bytes.
      setLength(position);
      if ( updateSize( 50, getBufSize()) ) {
         mc2dbg << "[SReqP]: Resized packet to " << getBufSize()
                << " bytes" << endl;
      }
      incWriteLong(position, mapID[i]);
   }
   incWriteLong( position, nbrLocations );
   incWriteLong( position, nbrMasks );
   incWriteLong( position, nbrCategories );
   incWriteLong( position, locationType );
   incWriteLong( position, regionsInMatches );
   incWriteLong( position, topRegion );
   incWriteByte( position, dbMask );
   DEBUG1({if (dbMask <= 0) {
      mc2log << error << "dbMask is 0!" << endl;
   }});
   incWriteString(position, zipCode);
   for ( i = 0 ; i < nbrMasks ; i++ ) { 
      incWriteLong( position, maskItemIDs[ i ] );
      // Double the size if there isn't room for 50 more bytes.
      setLength(position);
      if ( updateSize( 50, getBufSize()) ) {
         mc2dbg << "[SReqP]: Resized packet to " << getBufSize()
                << " bytes" << endl;
      }
   }
   for ( i = 0 ; i < nbrCategories ; i++ ) {
      incWriteLong( position, categories[ i ] );
      // Double the size if there isn't room for 50 more bytes.
      setLength(position);
      if ( updateSize( 50, getBufSize()) ) {
         mc2dbg << "[SReqP]: Resized packet to " << getBufSize()
                << " bytes" << endl;
      }
   }
   for ( i = 0 ; i < nbrLocations ; i++ ) {
      incWriteString( position, locations[ i ] );
      // Double the size if there isn't room for 50 more bytes.
      setLength(position);
      if ( updateSize( 50, getBufSize()) ) {
         mc2dbg << "[SReqP]: Resized packet to " << getBufSize()
                << " bytes" << endl;
      }
   }
   for ( i = 0 ; i < nbrMasks ; i++ ) { 
      incWriteString( position, maskNames[ i ] );
      // Double the size if there isn't room for 50 more bytes.
      setLength(position);
      if ( updateSize( 50, getBufSize()) ) {
         mc2dbg << "[SReqP]: Resized packet to " << getBufSize()
                << " bytes" << endl;
      }
   }

   // Write the bounding boxes
   incWriteLong(position, bboxes.size());
   for( vector<MC2BoundingBox>::const_iterator it = bboxes.begin();
        it != bboxes.end();
        ++it ) {
      incWriteBBox(position, *it);
   }
   
   return position;
}


int
OldSearchRequestPacket::readHeader( SearchTypes::SearchSorting& sortingType,
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
                                 uint32& regionsInMatches,
                                 StringTable::countryCode& topRegion,
                                 vector<MC2BoundingBox>& bboxes
                                 ) const
{
   sortingType = static_cast<SearchTypes::SearchSorting>
      (readLong(REQUEST_HEADER_SIZE));
   int numMapID = readLong(REQUEST_HEADER_SIZE+4);
   int position = REQUEST_HEADER_SIZE + numMapID*4 + 8;
   
   nbrLocations = incReadLong( position );
   nbrMasks = incReadLong( position );
   nbrCategories = incReadLong( position );
   locationType = incReadLong( position );
   regionsInMatches = incReadLong( position );
   topRegion = (StringTable::countryCode) incReadLong( position );
   dbMask = incReadByte( position );
   DEBUG1({if (dbMask <= 0) {
      mc2log << error << "dbMask is 0!" << endl;
   }});
   incReadString( position, zipCode);
   /* check
      nbrMasks
      nbrCategories
      nbrLocations
   */
   DEBUG1({
      if (nbrMasks > 255) {
         mc2log << warn << "SRP::readHeader: nbrMasks = "
                << nbrMasks << endl;
      }
      if (nbrCategories > 255) {
         mc2log << warn << "SRP::readHeader: nbrCategories = "
                << nbrCategories << endl;
      }
      if (nbrLocations > 255) {
         mc2log << warn << "SRP::readHeader: nbrLocations = "
                << nbrLocations << endl;
      }
   });

   // I've added "+1" to all the new[]:s here to avoid
   // strange errors when allocating/deallocating zero elements.
   masks = new uint32[ nbrMasks+1 ]; // might fail
   maskItemIDs = new uint32[ nbrMasks+1 ]; // might fail
   categories = new uint32[ nbrCategories+1 ]; // might fail
   uint32 i = 0;
   for ( i = 0 ; i < nbrMasks ; i++ ) {
      masks[ i ] = MAX_UINT32;
      maskItemIDs[ i ] = incReadLong( position );
   }
   for ( i = 0 ; i < nbrCategories ; i++ ) {
      categories[ i ] = incReadLong( position );
   }
   locations = new char*[ nbrLocations+1 ];
   for ( i = 0 ; i < nbrLocations ; i++ ) {
      incReadString( position, locations[ i ] );
   }
   maskNames = new char*[ nbrMasks+1 ];
   for ( i = 0 ; i < nbrMasks ; i++ ) {
      incReadString( position,  maskNames[ i ] );
   }
   DEBUG2({
      if (position > int(getBufSize())) {
         mc2log << warn << "position > getBufSize()" 
                << "position = " << position
                << "getBufSize() = " << getBufSize() << endl;
      }
   });

   // Read boundingboxes
   int nbrBBoxes = incReadLong(position);
   for( int i = 0; i < nbrBBoxes; ++i ) {
      bboxes.push_back(MC2BoundingBox());
      incReadBBox(position, bboxes.back());
   }
   
   // Align and return that position
   //AlignUtility::alignLong(position);
   return position;
}

int 
OldSearchRequestPacket::getHeaderEndPosition() const
{
   char* foo = NULL;

   // Initiate the position with the first fields.
   int position =
      REQUEST_HEADER_SIZE + 
      4 +                     // sortingType
      4 +                     // nbrMapIDs
      getNumMapID()*4 +       // mapIDs
      4 +                     // nbrLocations
      4 +                     // nbrMasks
      4 +                     // nbrCategories
      4 +                     // locationType
      4 +                     // regionsInMatches
      4 +                     // topRegion
      1;                      // dbMask
   
   // Add the size of the zipCode-string
   incReadString(position, foo);

   // Add the size occupied by the masks
   if ((getNbrMasks() > 0) || (getNbrCategories() > 0)) {
      AlignUtility::alignLong(position);
   }
   // Only itemIDs no real masks
   position += getNbrMasks() * 4;

   // Add the size occupied by the categories.
   position += getNbrCategories() * 4;

   // Add the size of the locationStrings
   int i;
   int nbrLocations = getNbrLocations();
   for ( i = 0 ; i < nbrLocations ; i++ ) {
      incReadString(position, foo);
   }

   // Add the size of the mask-strings
   int nbrMasks = getNbrMasks();
   for ( i = 0 ; i < nbrMasks ; i++ ) {
      incReadString(position,  foo);
   }
   
   // Read boundingboxes
   int nbrBBoxes = incReadLong(position);
   MC2BoundingBox bbox;
   for( int i = 0; i < nbrBBoxes; ++i ) {      
      incReadBBox(position, bbox);
   }   
   // Return the aligned position...
   //AlignUtility::alignLong(position);
   return position;
}

void OldSearchRequestPacket::dump( bool headerOnly, bool lookupIP ) const
{
   Packet::dump(true, lookupIP);
   DEBUG8(
      cout << "SEARCH REQUEST HEADER" << endl;
      cout << "Number of mapID: " << getNumMapID() << endl;
      if( getNumMapID() > 0 ){
         cout << "MapID(s):" << endl;
         for(uint32 i=0;i<getNumMapID(); i++ ){
            cout << getMapID(i);   
         }
         cout << endl;
      }
      /*const SortingT* sortType = getSortingType();
      cout << "Sorting type: " << (int)sortType << endl << endl;
      delete sortType;*/
   );
}

const char* OldSearchRequestPacket::getZipCode() const
{
   return  NULL;
}


uint32
OldSearchRequestPacket::getNbrLocations() const
{
   int nbrMapIDs = readLong(REQUEST_HEADER_SIZE + 4);
   int pos = REQUEST_HEADER_SIZE + 8 + nbrMapIDs*4;

   return incReadLong( pos );
}


//const char **
//OldSearchRequestPacket::getLocations() const
//{
//   int nbrMapIDs = readLong(REQUEST_HEADER_SIZE + 4);
//   int pos = REQUEST_HEADER_SIZE + 8 + nbrMapIDs*4;
//
//   uint32 nbrLocations = incReadLong( pos );
//   uint32 nbrMasks = incReadLong( pos );
//   uint32 nbrCategories = incReadLong( pos );
//   pos += nbrMasks * 4;
//   pos += nbrCategories * 4;
//   // locationType + regionsInMatches + topRegion + dbMask
//   pos += 4 + 4 + 4 + 1; 
//
//   char** locations = new char*[ nbrLocations ];
//   for ( uint32 i = 0 ; i < nbrLocations ; i++ ) {
//      incReadString( pos, locations[ i ] );
//   }
//   return (const char **)locations;
//}


uint32
OldSearchRequestPacket::getNbrMasks() const
{
   int nbrMapIDs = readLong(REQUEST_HEADER_SIZE + 4);
   int pos = REQUEST_HEADER_SIZE + 8 + nbrMapIDs*4;
   
   pos += 4; // Skip nbrLocations
   return incReadLong( pos );
}

uint32
OldSearchRequestPacket::getNbrCategories() const
{
   int nbrMapIDs = readLong(REQUEST_HEADER_SIZE + 4);
   int pos = REQUEST_HEADER_SIZE + 8 + nbrMapIDs * 4;

   pos += 8; // skip nbrLocations and nbrMasks
   return incReadLong( pos );
}

//const uint32*
//OldSearchRequestPacket::getMasks() const {
//   int nbrMapIDs = readLong(REQUEST_HEADER_SIZE + 4);
//   int pos = REQUEST_HEADER_SIZE + 8 + nbrMapIDs*4;
//   
//   pos += 4; // Skip nbrLocations
//   uint32 nbrMasks = incReadLong( pos );
//
//   pos += 4; // skip nbrCategories
//   // skip locationType, regionsInMatches and dbMask
//   pos += 4 + 4 + 4 + 1;
//   char* zipCode;
//   incReadString( pos, zipCode ); // skip zipcode
//  
//   uint32* masks = new uint32[ nbrMasks ];
//   for ( uint32 i = 0 ; i < nbrMasks ; i++ ) { 
//      masks[i] = incReadLong( pos );
//      pos += 4; // Skip maskItemIDs
//   }
//    
//   return masks;
//}

uint32 
OldSearchRequestPacket::getRegionsInMatches() const {
   int nbrMapIDs = readLong(REQUEST_HEADER_SIZE + 4);
   int pos = REQUEST_HEADER_SIZE + 8 + nbrMapIDs*4;

   pos += 4*4;  // skip nbrLocations, nbrMasks, nbrCategories and
               // locationType
   return incReadLong( pos );   
}


// ****** OverviewSearchRequestPacket *************************************
OverviewSearchRequestPacket::OverviewSearchRequestPacket( 
   uint16 packetID,
   uint16 requestID,
   uint32 nbrLocations,
   const char** locations,
   uint32 locationType,
   uint32 requestedLanguage,
   uint32 mapID,
   SearchTypes::StringMatching matching,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   bool uniqueOrFull,
   uint8 dbMask,
   uint8 minNbrHits,
   uint32 regionsInMatches,
   const ItemIDTree& idTree)
      : RequestPacket( MAX_PACKET_SIZE,
                       SEARCH_REQUEST_PRIO,
                       PACKETTYPE_OVERVIEWSEARCHREQUESTPACKET,
                       packetID,
                       requestID,
                       mapID ) 
{
   int pos = REQUEST_HEADER_SIZE;
   incWriteLong( pos, locationType );
   incWriteLong( pos, requestedLanguage );
   incWriteLong( pos, nbrLocations );
   incWriteLong( pos, MAX_UINT32 ); // map ID
   incWriteLong( pos, regionsInMatches );
   incWriteByte( pos, uniqueOrFull );
   incWriteByte( pos, matching );
   incWriteByte( pos, stringPart );
   incWriteByte( pos, sortingType );
   incWriteByte( pos, dbMask );
   incWriteByte( pos, minNbrHits );
   mc2dbg4 << "dbmask " << (int) dbMask << endl;
   DEBUG1({if (dbMask <= 0) {
      mc2log << error << "dbMask is 0!" << endl;
   }});
   
   for ( uint32 i = 0 ; i < nbrLocations ; i++ ) {
      incWriteString( pos, locations[ i ] );
   }
   idTree.save( this, pos );
   setLength( pos );
}


SearchTypes::StringMatching
OverviewSearchRequestPacket::getMatching() const {
   return static_cast<SearchTypes::StringMatching>
      (readByte( REQUEST_HEADER_SIZE + 21 ));
}

SearchTypes::StringPart
OverviewSearchRequestPacket::getStringPart() const {
   return static_cast<SearchTypes::StringPart>
      (readByte( REQUEST_HEADER_SIZE + 22 ));
}

SearchTypes::SearchSorting
OverviewSearchRequestPacket::getSortingType() const {
   return static_cast<SearchTypes::SearchSorting>
      (readByte( REQUEST_HEADER_SIZE + 23 ));
}

uint8
OverviewSearchRequestPacket::getDBMask() const 
{
   uint8 dbMask = readByte( REQUEST_HEADER_SIZE + 24 );
   DEBUG1({if (dbMask <= 0) {
      mc2log << error << "dbMask is 0!" << endl;
   }});
   return dbMask;
}

uint8
OverviewSearchRequestPacket::getMinNbrHits() const 
{
   uint8 minNbrHits = readByte( REQUEST_HEADER_SIZE + 25 );
   return minNbrHits;
}

const char**
OverviewSearchRequestPacket::getLocationStrings(ItemIDTree& idTree) const {
   uint32 nbrStrings = getNbrLocations();
   char** str = new char*[ nbrStrings ];
   char* tmp = NULL;
   int pos = REQUEST_HEADER_SIZE + 26;
   for ( uint32 i = 0 ; i < nbrStrings ; i++ ) {
      incReadString( pos, tmp );
      str[ i ] = tmp;
   }
   // Load the idtree too.
   idTree.load( this, pos );
   return (const char **)str;
}

uint32
OverviewSearchRequestPacket::getMapID() const
{
   return readLong( REQUEST_HEADER_SIZE + 12);
}


uint32
OverviewSearchRequestPacket::getLocationType() const {
   return readLong( REQUEST_HEADER_SIZE );
}

uint32
OverviewSearchRequestPacket::getRequestedLanguage() const {
   return readLong( REQUEST_HEADER_SIZE + 4 );
}

uint32 
OverviewSearchRequestPacket::getNbrLocations() const {
   return readLong( REQUEST_HEADER_SIZE + 8 );
}


bool
OverviewSearchRequestPacket::getUniqueOrFull() const {
   return (readByte( REQUEST_HEADER_SIZE + 20) != 0);
}


uint32 
OverviewSearchRequestPacket::getRegionsInMatches() const {
   return readLong( REQUEST_HEADER_SIZE + 16 );
}
