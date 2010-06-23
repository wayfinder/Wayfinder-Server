/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapModuleNotice.h"
#include "StringUtility.h"
#include "GfxDataFactory.h"
#include "StringTable.h"
#include "MapBits.h"

MapModuleNotice::MapModuleNotice(uint32 mID, 
                                 GfxData* mapGfxData,
                                 uint32 creationTime) : MapNotice(mID)
{
   m_mapName = StringUtility::newStrDup("");
   m_gfxData = mapGfxData;
   m_countryCode = StringTable::SWEDEN_CC;
   m_creationTime = creationTime;
}

MapModuleNotice::MapModuleNotice(DataBuffer* dataBuffer,
                                 int indexdbversion,
                                 uint32 mapSet)
{
   m_mapName = NULL;
   mc2dbg4 << "[MapModuleNotice] - created with DataBuffer*" << endl;

   int version = 0;
   if ( indexdbversion > 0 ) {
      version = dataBuffer->readNextLong();
      mc2dbg4 << "[MapModuleNotice] - version = "
             << version << endl;
   } else {
      mc2dbg4 << "[MapModuleNotice] - not reading version" << endl;
   }

   m_mapID = dataBuffer->readNextLong();
   if (mapSet != MAX_UINT32)
      m_mapID = MapBits::getMapIDWithMapSet(m_mapID, mapSet);

   m_countryCode = StringTable::countryCode(dataBuffer->readNextLong());
   m_creationTime = dataBuffer->readNextLong();
   mc2dbg4 << "Read creationtime for map " << m_mapID << ":" 
           << m_creationTime << endl;

   uint32 nbrStrings = dataBuffer->readNextShort();
   mc2dbg4 << "[MMN]: Number of strings = " << nbrStrings << endl;
   // Read status for compatibility.
   uint8 currentStatus = dataBuffer->readNextByte();
   mc2dbg8 << "Current status = " << uint32(currentStatus) << endl;
   
   uint32 nbrNeighbours = dataBuffer->readNextByte();
   // Read in the neighbours.
   m_neighbourMaps.clear();
   for (uint32 i=0; i < nbrNeighbours; i++) {
      m_neighbourMaps.insert( dataBuffer->readNextLong() );
   }

   // OK to make the name-vector static. No adds possible.
   // I have removed the name-array so we will not save
   // the names.
   // Read the strings for backwards compatibility.
   mc2dbg4 << "nbr Strings = " << nbrStrings << endl;
   for (uint32 i = 0; i < nbrStrings; i++) {
      const char* tmpStr = dataBuffer->readNextString();
      mc2dbg4 << "String:" << tmpStr << endl;
   }

   m_gfxData = GfxDataFactory::create( *dataBuffer );
   // Update the bounding box so that it can be used instead
   // of the slow getBounding box of the gfxdata.
   m_gfxData->getMC2BoundingBox(m_bbox);

   if ( version > 0 ) {
      // We save the mapname in versions > 0
      setMapName( dataBuffer->readNextString() );
   } else {
      m_mapName = StringUtility::newStrDup("");
   }
   mc2dbg4 << "[MMN]: Loaded map name \"" << m_mapName
           << "\" for map " << m_mapID << endl;
}

MapModuleNotice::~MapModuleNotice()
{
   delete m_gfxData;
   delete [] m_mapName;
}

uint32 MapModuleNotice::getSizeInDataBuffer(int version) const
{

   uint32 result=0;

   result+=4; //This compensates for alignment.
   
   if (version > 0){
      result+=4;
   }
   result+=4;
   result+=4;
   result+=4;
   result+=2;
   result+=1;
   result+=1;
   for( set<uint32>::const_iterator it = m_neighbourMaps.begin();
        it != m_neighbourMaps.end();
        ++it )
   {
      result+=4;
   }
   result+=m_gfxData->getSizeInDataBuffer();
   if ( version > 0 ) {
      result+=strlen(m_mapName)+1;
   }

   return result;
}

bool
MapModuleNotice::save(DataBuffer* dataBuffer, int version)
{
   DataBufferChecker dbc(*dataBuffer, "MapModuleNotice::save");
   dbc.assertRoom(getSizeInDataBuffer(version));


   mc2dbg << "Saves MapModuleNotice" << endl;

   if ( version > 0 ) {
      // Version of MapModuleNotice on disk.
      // Differs from the version of index.db, but does
      // not exist in index.db version 0
      dataBuffer->writeNextLong( 1 );
   }
   dataBuffer->writeNextLong(m_mapID);

   dataBuffer->writeNextLong(m_countryCode);
   dataBuffer->writeNextLong(m_creationTime);

   dataBuffer->writeNextShort(0); // Number of strings
   dataBuffer->writeNextByte(0);  // Current status. Not used.
   dataBuffer->writeNextByte(m_neighbourMaps.size());

   mc2dbg4 << "[MMN]: Saving mapnotice with mapID " << m_mapID << endl;
   mc2dbg4 << ", nbrStrings = 0, nbrNeighbours = "
           << m_neighbourMaps.size() << endl; 

   mc2dbg4 << "       m_neighbourMaps: [";

   // Save neighbour maps
   for( set<uint32>::const_iterator it = m_neighbourMaps.begin();
        it != m_neighbourMaps.end();
        ++it ) {
      dataBuffer->writeNextLong(*it);
      mc2dbg4 << *it << " ";
   }
   mc2dbg4 << "]" << endl;

   // Save the strings.
   // Removed

   // Save the GfxData
   m_gfxData->save( *dataBuffer );
   MC2BoundingBox bbox;
   m_gfxData->getMC2BoundingBox(bbox);
   mc2dbg << "Saved gfx with bbox:" << endl;
   bbox.dump();


   if ( version > 0 ) {
      mc2dbg4 << "[MMN]: Saving map name \"" << m_mapName << "\" for map id "
             << m_mapID << endl;      
      dataBuffer->writeNextString(m_mapName);
   }

   return (true);
}

void
MapModuleNotice::setMapName(const char* name)
{
   delete[] m_mapName;
   m_mapName = StringUtility::newStrDup( (name != NULL ) ? name : "null" );
}


void
MapModuleNotice::setCountryMapName(const char* name)
{
   MC2String countryStr( countryNamePrefix );
   countryStr += (name != NULL ) ? name : "null";
   setMapName( countryStr.c_str() );
}

const char*
MapModuleNotice::countryNamePrefix = "country_";


void MapModuleNotice::setCountryCode( const CountryCode& country) {
   m_countryCode = country;
}

CountryCode MapModuleNotice::getCountryCode() const {
   return static_cast< StringTable::countryCode >( m_countryCode );
}
