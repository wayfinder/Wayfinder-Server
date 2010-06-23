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

#include "GenericMapHeader.h"

#include "DataBufferCreator.h"
#include "DataBuffer.h"
#include "DataBufferUtil.h"
#include "StringUtility.h"
#include "STLStringUtility.h"
#include "StringTable.h"
#include "TempFile.h"
#include "DebugClock.h"
#include "BitUtility.h"
#include "MapBits.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <memory>

// Number of initial item allocators = ItemTypes::itemType 0-25 (including 
// routeableItem which really is no allocator...)
const uint32 GenericMapHeader::numberInitialItemTypeAllocators = 26;

void
GenericMapHeader::setMapName( const char* name )
{
   delete [] m_name;
   m_name = StringUtility::newStrDup( name );
}

void
GenericMapHeader::setMapOrigin( const char* origin )
{
   delete  [] m_origin;
   m_origin = StringUtility::newStrDup( origin );
}

void
GenericMapHeader::initEmptyHeader(uint32 id)
{
   m_mapID = id;
   m_pathname = NULL;
   m_filename = NULL;
   m_name   = StringUtility::newStrDup("");
   m_origin = StringUtility::newStrDup("");
   
   // Default values for the members describing data about all the items
   m_country = StringTable::SWEDEN_CC;
   m_driveOnRightSide = true;
   m_currency = NULL;
   m_copyrightString = NULL;
   
   m_creationTime = MAX_UINT32;
   m_groupsInLocationNameOrder = false;
   m_trueCreationTime = MAX_UINT32;
   m_waspTime = MAX_UINT32;
   m_dynamicExtradataTime = MAX_UINT32;

   m_utf8Strings = false;
   m_mapFiltered = false;
   m_mapGfxDataFiltered = false;
   m_mapCountryDir = StringUtility::newStrDup("");
   m_loadedVersion = 0;
}

GenericMapHeader::GenericMapHeader(uint32 mapID)
{
   initEmptyHeader(mapID);
}

GenericMapHeader::GenericMapHeader(uint32 mapID, const char *path)
{
   initEmptyHeader(mapID);
   
   // Set the filename of the map
   setFilename(path);
}

GenericMapHeader::~GenericMapHeader()
{
   delete [] m_name;
   DEBUG_DEL(mc2dbg << "~GenericMapHeader name destr." << endl );
   
   delete [] m_origin;
   DEBUG_DEL(mc2dbg << "~GenericMapHeader origin destr." << endl );
   
   delete [] m_filename;
   DEBUG_DEL(mc2dbg << "~GenericMapHeader filename destr." << endl );

   delete [] m_pathname;
   DEBUG_DEL(mc2dbg << "~GenericMapHeader pathname destr." << endl );

   delete [] m_copyrightString;
   DEBUG_DEL(mc2dbg << "~GenericMapHeader copyrightstr destr." << endl );

   delete[] m_mapCountryDir;
   
}

bool
GenericMapHeader::save( const char* filename, bool updateCreationTime )
{
   int un_res = unlink( filename );
   if ( un_res == 0 ) {
      mc2dbg << "[GMH]: Removed " << MC2CITE( filename ) << endl;
   } else {
      if ( errno == ENOENT ) {
         // Ok. No file exists
      } else {
         mc2log << error << "[GMH]::save could not remove "
                << filename << endl;
         return false;
      }
   }
   
   MC2String dirname = STLStringUtility::dirname( filename );
   
   char tempFilename[64];
   sprintf( tempFilename, "GMHsave0x%x_", m_mapID );
   TempFile tempFile( tempFilename, dirname, filename );

   if ( ! tempFile.ok() ) { 
      mc2log << error << "In GenericMapHeader::save(). " << endl;
      return false;
   }

   mc2dbg << "[GMH]: Writing to temp file " 
          <<  MC2CITE( tempFile.getTempFilename() ) << endl;
   

   m_updateCreationTime = updateCreationTime;
   bool retVal = internalSave( tempFile.getFD() );

   // Unfortunately internalSave does not return false
   // if the disk is full....
   if ( retVal ) {
      // Chmod the file
      if ( fchmod( tempFile.getFD(), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ) == -1 ) {
         mc2dbg << "[GMH]: Failed to chmod file. Error: " 
                << strerror( errno ) << endl;
      }
   }  else {
      mc2log << error << "[GMH]::save error when saving - removing "
             << tempFile.getTempFilename() << endl;

      tempFile.setFailed( true );
   }


   return retVal;
}

bool
GenericMapHeader::save()
{
   // Save with default filename.
   return save( m_filename );
}

bool 
GenericMapHeader::load()
{
   mc2dbg << "GenericMap::load()" << endl;
   mc2dbg2 << "   open infile " << m_filename << endl;
   //mc2dbg << "[GMH]: m_path: " << m_pathname << endl;
   DebugClock loadClock;

   // We must at least have a valid filename...
   if ( m_filename == NULL || strlen( m_filename ) == 0) {
      return false;
   }

   MC2String filename = m_filename;

   std::auto_ptr<DataBuffer> db;

   //
   // There are four ways to load data to map
   // and they are tried in the following order:
   // 1) Fetch from internet with specified URL
   // 2) Load from local disk 
   // 3) Load from local disk .bz2 file
   // 4) Load from local disk .gz file
   //

   DataBufferCreator dbcreator; // less typing

   // if there is no path then use MAP_PATH and fallback on URL
   if ( m_pathname == NULL || strlen( m_pathname ) == 0) {
      mc2dbg << "[GMH] No path specified, trying to resolv "
             << "from MAP_PATH and MAP_PATH_URL" << endl;
      
      try {
         db.reset( DataBufferCreator::loadMapOrIndexDB( m_filename ) );
      } catch ( MC2String& e ) {
         mc2log << warn << e << endl;
      }
   } else {

      if ( strstr( filename.c_str(), "http://" ) == filename.c_str() ) {
         db.reset( dbcreator.loadFromURL( filename.c_str() ) );
      } else {
         // do normal load, we set the path ourself
         db.reset( dbcreator.loadFromFile( filename.c_str() ) );
      }
   }

   //
   // If one of the above load methods succeeded
   // we should have a DataBuffer -> load it
   //
   bool retVal = db.get() ? internalLoad( *db.get() ) : false;

   if ( retVal ){
      mc2log << info << "Map 0x" << hex << getMapID() << dec << "(" 
             << getMapID() << ") loaded in " 
             << loadClock << "." << endl; 
   }
   return retVal;
}

bool
GenericMapHeader::internalLoad(DataBuffer& dataBuffer)
{
   CLOCK_MAPLOAD(DebugClock loadClock);
   mc2dbg4 << "GenericMapHeader::internalLoad" << endl;
   // ***************************************************************
   //                                Read the general map information
   // ***************************************************************
   uint32 currentMapID = dataBuffer.readNextLong();
   // do check without map set bits
   if (currentMapID != MapBits::getMapIDWithoutMapSet( m_mapID )) {
      mc2log << fatal << "GenericMapHeader::createFromDataBuffer, currentMapID: " 
             << prettyMapIDFill(currentMapID) << " does not match filename, mapID: " 
             << prettyMapIDFill(m_mapID) << ". EXITING!" << endl;
      exit(1);
   }
   m_totalMapSize = dataBuffer.readNextLong(); // WARNING: Not set in file!!!
   mc2dbg << "[GMH]: Total map size variable = " << m_totalMapSize
          << " and size of DataBuffer = " << dataBuffer.getBufferSize()
          << endl;
   
   // Read the creationtime of this map
   m_creationTime = dataBuffer.readNextLong();

   // Read some data about all the items in this map
   m_country = StringTable::countryCode(dataBuffer.readNextLong());
   
   byte flagByte = dataBuffer.readNextByte();
   byte nbrNativeLanguages = dataBuffer.readNextByte();
   byte nbrCurrencies = dataBuffer.readNextByte();
   m_loadedVersion= dataBuffer.readNextByte();

   if ( m_loadedVersion < 3 ) {
      m_driveOnRightSide = (flagByte != 0);
   } else {
      m_driveOnRightSide          = BitUtility::getBit( flagByte, 0 );
      m_groupsInLocationNameOrder = BitUtility::getBit( flagByte, 1 );
   }
   
   m_nbrAllocators = dataBuffer.readNextLong();
   mc2dbg << "Nbr allocators: " << m_nbrAllocators << endl;

   // The languages
   m_nativeLanguages.resize( nbrNativeLanguages );
   for (NativeLanguageIndexes::size_type i=0; i<nbrNativeLanguages; i++) {
      m_nativeLanguages[ i ] = 
         static_cast<NativeLanguageIndexes::value_type>
         ( dataBuffer.readNextLong() );
   }

   // The currencies
   m_currency = new Vector(nbrCurrencies);
   for (uint32 i=0; i<nbrCurrencies; i++) {
      m_currency->addLast(dataBuffer.readNextLong());
   }

   // Variable header: 
   delete [] m_name;
   delete [] m_origin;
   delete [] m_mapCountryDir;
   switch ( m_loadedVersion ) {
      case ( 0 ) :
         m_name = StringUtility::newStrDup( "" );
         m_origin = StringUtility::newStrDup( "" );
         m_trueCreationTime = MAX_UINT32;
         m_waspTime = MAX_UINT32;
         m_dynamicExtradataTime = MAX_UINT32;
         m_utf8Strings = false;
         m_mapFiltered = false;
         m_mapGfxDataFiltered = false;
         m_mapCountryDir = StringUtility::newStrDup("");

         break;
      case ( 1 ) : {
         uint32 offset = dataBuffer.getCurrentOffset();
         uint32 variableHeaderSize = dataBuffer.readNextLong();
         
         m_name = StringUtility::newStrDup( dataBuffer.readNextString() );
         m_origin = StringUtility::newStrDup( "" );
         m_trueCreationTime = MAX_UINT32;
         m_waspTime = MAX_UINT32;
         m_dynamicExtradataTime = MAX_UINT32;
         m_utf8Strings = false;
         m_mapFiltered = false;
         m_mapGfxDataFiltered = false;
         m_mapCountryDir = StringUtility::newStrDup("");

         dataBuffer.readPastBytes( variableHeaderSize - 
                                  ( dataBuffer.getCurrentOffset() - offset ) );
       } break;         
      case ( 2 ) :
      case ( 3 ) : {
         uint32 offset = dataBuffer.getCurrentOffset();
         uint32 variableHeaderSize = dataBuffer.readNextLong();
         
         m_name = StringUtility::newStrDup( dataBuffer.readNextString() );
         m_origin = StringUtility::newStrDup( dataBuffer.readNextString() );
         m_trueCreationTime = MAX_UINT32;
         m_waspTime = MAX_UINT32;
         m_dynamicExtradataTime = MAX_UINT32;
         m_utf8Strings = false;
         m_mapFiltered = false;
         m_mapGfxDataFiltered = false;
         m_mapCountryDir = StringUtility::newStrDup("");

         dataBuffer.readPastBytes( variableHeaderSize - 
                                  ( dataBuffer.getCurrentOffset() - offset ) );
       } break;
      case ( 4 ) : {
         uint32 offset = dataBuffer.getCurrentOffset();
         uint32 variableHeaderSize = dataBuffer.readNextLong();
         
         m_name = StringUtility::newStrDup( dataBuffer.readNextString() );
         m_origin = StringUtility::newStrDup( dataBuffer.readNextString() );
         m_trueCreationTime = dataBuffer.readNextLong();
         m_waspTime = dataBuffer.readNextLong();
         m_dynamicExtradataTime = dataBuffer.readNextLong();
         m_utf8Strings = false;
         m_mapFiltered = false;
         m_mapGfxDataFiltered = false;
         m_mapCountryDir = StringUtility::newStrDup("");

         dataBuffer.readPastBytes( variableHeaderSize - 
                                  ( dataBuffer.getCurrentOffset() - offset ) );
       } break;
      case (5) :
      case (6) :  // GenericMap::m_userRightsTable
      case (7) :  // GenericMap::m_adminAreaCentres
      default : {
         uint32 offset = dataBuffer.getCurrentOffset();
         uint32 variableHeaderSize = dataBuffer.readNextLong();
         
         m_name = StringUtility::newStrDup( dataBuffer.readNextString() );
         m_origin = StringUtility::newStrDup( dataBuffer.readNextString() );
         m_trueCreationTime = dataBuffer.readNextLong();
         m_waspTime = dataBuffer.readNextLong();
         m_dynamicExtradataTime = dataBuffer.readNextLong();
         m_utf8Strings = dataBuffer.readNextBool();
         m_mapFiltered = dataBuffer.readNextBool();
         m_mapGfxDataFiltered = dataBuffer.readNextBool();
         m_mapCountryDir = 
            StringUtility::newStrDup( dataBuffer.readNextString() );

         dataBuffer.readPastBytes( variableHeaderSize - 
                                  ( dataBuffer.getCurrentOffset() - offset ) );
       } break;
   }

   // Read string with copyright-information for images of this map
   m_copyrightString = StringUtility::newStrDup(dataBuffer.readNextString());

   CLOCK_MAPLOAD(mc2log << "[" << m_mapID << "] Header loaded in "
                 << loadClock
                 << ", m_loadedVersion=" << int(m_loadedVersion) << endl );
   return true;
}

bool
GenericMapHeader::internalSave(int outfile)
{
   DebugClock saveClock;

   // ***************************************************************
   //                                Save the general map information
   // ***************************************************************
   byte nbrNativeLanguages = m_nativeLanguages.size();
   
   byte nbrCurrencies = 0;
   if (m_currency != NULL)
      nbrCurrencies = (byte) m_currency->getSize();

   // version: 7
   byte version = 7;

   // Calculate variable header size.
   uint32 nameAndOrigLen = ( strlen(m_name)+1 + strlen(m_origin)+1 );
   AlignUtility::alignLong( nameAndOrigLen ); // Align because followed by long
   uint32 variableHeaderSize = 
      4 +         // varSize
      nameAndOrigLen +
      4 + 4 + 4 + // (trueCreation + wasp + ed)
      1 +         // utf8 strings bool
      1 +         // map filtered bool
      1 +         // map gfx data filtered bool
      strlen( m_mapCountryDir ) + 1;

   AlignUtility::alignLong( variableHeaderSize );

   // The number of allocators, all itemTypes, GfxDatas, Nodes and 
   // Connections etc:
   //    numberInitialItemTypeAllocators(26) + 
   //    GfxDatas(1) + Nodes(1) + Connections(1) +
   //    GfxDataSingleSmallPoly(1) + GfxDataSingleLine(1) +
   //    GfxDataSinglePoint(1) + GfxDataMultiplePoints(1) +
   //    simpleItems(1) + coordinates + lanes + categories + SignPosts
   m_nbrAllocators = uint16(numberInitialItemTypeAllocators)+12;

   // 256 for the copyright string
   DataBuffer* dataBuffer = new DataBuffer(20 + 4*nbrNativeLanguages + 
                                           4*nbrCurrencies +
                                           variableHeaderSize +
                                           4 + 8*m_nbrAllocators + 256);
   dataBuffer->fillWithZeros();

   dataBuffer->writeNextLong(m_mapID);
   dataBuffer->writeNextLong(0);    // Total size to be filled in later!!!

   // Save the current time
   if ( m_updateCreationTime ) {
      m_creationTime = TimeUtility::getRealTime();
   } else {
      mc2log << info << "[GMH]: Not updating the creation time" << endl;
   }
   dataBuffer->writeNextLong(m_creationTime);

   // Save some data about all the items in this map
   dataBuffer->writeNextLong( (uint32) m_country);

   byte flagByte = 0;
   flagByte = BitUtility::setBit( flagByte, 0, m_driveOnRightSide);
   flagByte = BitUtility::setBit( flagByte, 1, m_groupsInLocationNameOrder);
   
   dataBuffer->writeNextByte(flagByte);

   dataBuffer->writeNextByte(nbrNativeLanguages);
   dataBuffer->writeNextByte(nbrCurrencies);
   dataBuffer->writeNextByte(version);

   dataBuffer->writeNextLong(m_nbrAllocators);

   // The languages
   for ( NativeLanguageIndexes::size_type i = 0;
         i < nbrNativeLanguages; ++i ) {
      dataBuffer->writeNextLong( m_nativeLanguages[ i ] );
   }

   // The currencies
   for (uint32 i=0; i<nbrCurrencies; i++) {
      dataBuffer->writeNextLong(m_currency->getElementAt(i));
   }

   // "Calculate" true creation time,
   // it is set the first time this map is saved.
   if (m_trueCreationTime == MAX_UINT32)
      m_trueCreationTime = TimeUtility::getRealTime();

   // Variable header
   // Currently holds the map name, origin, and 3 times, and ..
   dataBuffer->writeNextLong( variableHeaderSize );
   dataBuffer->writeNextString( m_name );
   dataBuffer->writeNextString( m_origin );  
   dataBuffer->writeNextLong( m_trueCreationTime );
   dataBuffer->writeNextLong( m_waspTime );
   dataBuffer->writeNextLong( m_dynamicExtradataTime );
   // version: 5
#ifdef MC2_UTF8
   m_utf8Strings = true;
#else
   m_utf8Strings = false;
#endif
   dataBuffer->writeNextBool( m_utf8Strings );
   dataBuffer->writeNextBool( m_mapFiltered );
   dataBuffer->writeNextBool( m_mapGfxDataFiltered );
   dataBuffer->writeNextString( m_mapCountryDir );
   // version: 6 (no changes in header, but in allocator handling)


   dataBuffer->alignToLongAndClear();
   // End of variable header.


   // String with copyright-information for images of this map
   
   const char* copyrightStringToSave = m_copyrightString;   
   if ( copyrightStringToSave == NULL ) {
      copyrightStringToSave = "© 2010";
   }
   
   dataBuffer->writeNextString( copyrightStringToSave );
   dataBuffer->alignToLongAndClear();

   mc2dbg << "[GenricMapHeader]: Header saved in "
          << saveClock
          << ", version=" << int(version) << endl;

   DataBufferUtil::saveBuffer( *dataBuffer, outfile );

   delete dataBuffer;
   
   return true;
}


void 
GenericMapHeader::setMapID(uint32 newMapID) 
{
   m_mapID = newMapID;
   char* tmpStr = new char[strlen(m_pathname)+1];
   strcpy(tmpStr, m_pathname);
   setFilename(tmpStr);
   delete [] tmpStr;
}

void 
GenericMapHeader::setCountryCode(StringTable::countryCode countryName)
{
   m_country = countryName;
}

void 
GenericMapHeader::setDrivingSide(bool driveRight)
{
   m_driveOnRightSide = driveRight;
}


void GenericMapHeader::addNativeLanguage(LangTypes::language_t lang) {
   m_nativeLanguages.push_back( lang );
}

void GenericMapHeader::clearNativeLanguages() {
   m_nativeLanguages.clear();
}

void 
GenericMapHeader::addCurrency(StringTable::stringCode cur )
{
   // If there is no currency-array we create it here
   if (m_currency == NULL)
      m_currency = new Vector(2, 2);

   // Add the currency to the array
   m_currency->addLast(cur);

}

void 
GenericMapHeader::clearCurrencies()
{
   if (m_currency != NULL)
      m_currency->reset();
}

void 
GenericMapHeader::setCopyrightString(const char* copyright)
{
   delete [] m_copyrightString;
   m_copyrightString = StringUtility::newStrDup(copyright);
}

uint32 GenericMapHeader::getMapVersion() {
   return 17;
}

MC2String GenericMapHeader::getFilenameFromID( int id ) {
   char buff[ 256 ];

   sprintf( buff, "%09x-%d.m3",  
            MapBits::getMapIDWithoutMapSet( id ), 
            getMapVersion() );
   return MC2String( buff );
}

int GenericMapHeader::getMapVersionFromFilename( const MC2String& filename ) {
   int version;
   int mapID;
   MC2String str = STLStringUtility::basename( filename );
   if (sscanf( str.c_str(), "%09x-%d.m3", &mapID, &version ) != 2 ){
      return -1;
   }

   return version;
}

void
GenericMapHeader::setFilename( const char* path )
{
   delete [] m_pathname;

   if ( path == NULL ) {
      m_pathname = StringUtility::newStrDup("");
   } else {
      m_pathname = new char[ strlen(path) + 1 ];  // path + \0
      strcpy( m_pathname, path );
   }

   delete [] m_filename; 

   string filename = path ? path : "";

   filename += getFilenameFromID( m_mapID );

   m_filename = StringUtility::newStrDup( filename.c_str() );

   mc2dbg4 << "Filename set to " << m_filename << endl;
}

void GenericMapHeader::setTrueCreationTime( uint32 time ) {
   if ( time != MAX_UINT32 ) {
      m_trueCreationTime = time;
   } else {
      m_trueCreationTime = TimeUtility::getRealTime();
   }
}

void GenericMapHeader::setWaspTime( uint32 time ) {
   if ( time != MAX_UINT32 ) {
      m_waspTime = time;
   } else {
      m_waspTime = TimeUtility::getRealTime();
   }
}

void GenericMapHeader::setDynamicExtradataTime( uint32 time ) {
   if ( time != MAX_UINT32 ) {
      m_dynamicExtradataTime = time;
   } else {
      m_dynamicExtradataTime = TimeUtility::getRealTime();
   }
}
