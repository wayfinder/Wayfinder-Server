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

#include "OldGenericMapHeader.h"

#include "DataBuffer.h"
#include "StringUtility.h"
#include "ByteBuffer.h"
#include "StringTable.h"
#include "ItemTypes.h"
#include "TempFile.h"
#include "STLStringUtility.h"
#include "BitUtility.h"
#include "TimeUtility.h"
#include "MapBits.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

// Number of initial item allocators = ItemTypes::itemType 0-25 (including 
// routeableItem which really is no allocator...)
const uint32 OldGenericMapHeader::numberInitialItemTypeAllocators = 26;


void
OldGenericMapHeader::setMapName( const char* name )
{
   delete [] m_name;
   m_name = StringUtility::newStrDup( name );
}

void
OldGenericMapHeader::setMapOrigin( const char* origin )
{
   delete  [] m_origin;
   m_origin = StringUtility::newStrDup( origin );
}

OldGenericMapHeader*
OldGenericMapHeader::createMap(const char* mcmName)
{
   // Extract the ID and path from mcmName
   char path[256];
   path[0] = '\0';
   uint32 mapID;
   bool ok = false;

   // Find the last '/'
   char* slashPtr = StringUtility::strrchr(mcmName, '/');
   if (slashPtr == NULL) {
      // No slash, se if id + ".mcm" is provided
      if (sscanf(mcmName, "%x.mcm", &mapID) == 1) {
         strcat(path, "./");
         ok = true;
      }
   } else {
      // Got a slash, get mapID
      if (sscanf(slashPtr, "/%x.mcm", &mapID) == 1) {
         // Got mapID, get path
         int n = slashPtr-mcmName+1;
         if (strncpy(path, mcmName, n) != NULL) {
            path[n] = '\0';
            ok = true;
         }
      }
   }
   
   mc2dbg << "   mcmName=" << mcmName << endl;
   mc2dbg << "   path=" << path << ", mapID=" << mapID << endl;

   OldGenericMapHeader* result = NULL;
   if (ok) {
      // Create from other createMap-method
      result = new OldGenericMapHeader(mapID, path);
      result->load();
   }
   return result;
}

void
OldGenericMapHeader::initEmptyHeader(uint32 id)
{
   m_mapID = id;
   m_pathname = NULL;
   m_filename = NULL;
   m_name   = StringUtility::newStrDup("");
   m_origin = StringUtility::newStrDup("");
   
   // Default values for the members describing data about all the items
   m_country = StringTable::SWEDEN_CC;
   m_driveOnRightSide = true;
   m_nativeLanguages = NULL;
   m_currency = NULL;
   m_copyrightString = NULL;
   
   m_creationTime = MAX_UINT32;
   m_groupsInLocationNameOrder = false;
   m_trueCreationTime = MAX_UINT32;
   m_waspTime = 0;
   m_dynamicExtradataTime = MAX_UINT32;

   m_utf8Strings = false;
   m_mapFiltered = false;
   m_mapGfxDataFiltered = false;
   m_mapCountryDir = StringUtility::newStrDup("");
   m_loadedVersion = 0;
}

OldGenericMapHeader::OldGenericMapHeader(uint32 mapID)
{
   initEmptyHeader(mapID);
}

OldGenericMapHeader::OldGenericMapHeader(uint32 mapID, const char *path)
{
   initEmptyHeader(mapID);
   
   // Set the filename of the map
   setFilename(path);
}

OldGenericMapHeader::~OldGenericMapHeader()
{
   delete [] m_name;
   DEBUG_DEL(mc2dbg << "~OldGenericMapHeader name destr." << endl );
   
   delete [] m_origin;
   DEBUG_DEL(mc2dbg << "~OldGenericMapHeader origin destr." << endl );
   
   delete [] m_filename;
   DEBUG_DEL(mc2dbg << "~OldGenericMapHeader filename destr." << endl );

   delete [] m_pathname;
   DEBUG_DEL(mc2dbg << "~OldGenericMapHeader pathname destr." << endl );

   delete [] m_copyrightString;
   DEBUG_DEL(mc2dbg << "~OldGenericMapHeader copyrightstr destr." << endl );

   delete[] m_mapCountryDir;
   
}

bool
OldGenericMapHeader::save()
{
   // Create temp file, with real file path.
   MC2String dirname = STLStringUtility::dirname( m_filename );
   char tempFilename[64];
   sprintf( tempFilename, "tmpsave0x%x_", m_mapID );
   TempFile* tempFile = new TempFile( tempFilename, dirname, m_filename );
   if ( ! tempFile->ok() ) { 
      mc2log << error << "OldGenericMapHeader::save() tmpFile failed." << endl;
      MC2_ASSERT(false);
      return false;
   }
   mc2dbg << "Writing to temp file " 
          <<  MC2CITE( tempFile->getTempFilename() ) << endl;
   

   // Save content of the map.
   bool retVal = internalSave( tempFile->getFD() );
   if ( retVal ) {
      // Chmod the file
      if ( fchmod( tempFile->getFD(), 
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH ) == -1 ) {
         mc2dbg << error << "Failed to chmod file. Error: " 
                << strerror( errno ) << endl;
         MC2_ASSERT(false);
      }
   }  
   else {
      mc2log << error << "Save error when saving - removing "
             << tempFile->getTempFilename() << endl;
      tempFile->setFailed( true );
      MC2_ASSERT(false);
   }


   // Get the size written.
   off_t curFilePos = lseek(tempFile->getFD(), 0, SEEK_CUR);
   if ( curFilePos == (off_t)-1 ){
      mc2log << error << "OldGenericMapHeader::save falied to get file pos" 
             << endl;
      MC2_ASSERT(false);
   }

   // Remove the file if it already exists
   int un_res = unlink( m_filename );
   if ( un_res == 0 ) {
      mc2dbg << "Removed " << MC2CITE( m_filename ) << endl;
   } 
   else {
      if ( errno == ENOENT ) {
         // Ok. No file exists
      } 
      else {
         mc2log << error << "Save could not remove " << m_filename << endl;
         MC2_ASSERT(false);
         return false;
      }
   }
   
   // This makes the real file get written, by moving the temp file.
   delete tempFile;
   tempFile = NULL;


   mc2log << info << "Map 0x" << hex << getMapID() << dec << "(" 
          << getMapID() << ") "<< MC2CITE(m_name) << " saved "
          << curFilePos << " bytes." << endl; 

   return (retVal);
}

DataBuffer* 
OldGenericMapHeader::getDataBufferFromProgram(const char* command)
{
   FILE* program = popen(command, "r");
   if ( program == NULL ) {
      return NULL;
   }

   ByteBuffer byteBuffer;
   while ( !feof(program) ) {
      const int bufSize = 400 * 1024 * 1024;
      byte* buffer = new byte[bufSize];
      int readSize = fread( buffer, 1, bufSize, program);
      byteBuffer.add(buffer, readSize);
   }
   
   int progRes = pclose(program);
   
   if ( progRes == 0 && byteBuffer.getNbrBytes() > 2 ) {
      DataBuffer* db = new DataBuffer(byteBuffer.getNbrBytes());
      byteBuffer.read(db->getBufferAddress(),
                      byteBuffer.getNbrBytes());
      mc2dbg << "[GM]: Read " << db->getBufferSize()
             << " bytes from " << command << endl;               
      return db;
   } else {
      mc2log << error << "[GMap]: "
             << command << " returned to few bytes"
             << endl;
      return NULL;
   }
}


bool 
OldGenericMapHeader::load()
{
   mc2dbg4 << "OldGenericMap::load()" << endl;
   mc2dbg2 << "   open infile " << m_filename << endl;
   
   /*
   ifstream infile(m_filename, ios::in | ios::binary);
   if(!infile) {
      mc2log << error << here << " File error, " << m_filename << endl;
      perror("   System error is");
      return(false);
   }
   */
   
   uint32 startTime = TimeUtility::getCurrentMicroTime();
   bool retVal = false;

   if ( (m_filename!= NULL) && (strlen(m_filename) > 0)) {
      // Create a memory-mapped DataBuffer
      DataBuffer dataBuffer;
      if (dataBuffer.memMapFile(m_filename)) {
         retVal = internalLoad(dataBuffer);
      } else {         
         // Try bunzip
         //        Also try gzip         
         char* command = new char[strlen(m_filename)+1+500];
         sprintf(command, "bunzip2 -c %s.bz2", m_filename);
         mc2dbg << "[GMap]: Trying " << command << endl;
         DataBuffer* db = getDataBufferFromProgram(command);
         if ( db == NULL ) {
            // Try gunzip
            sprintf(command, "gunzip -c %s.gz", m_filename);
            mc2dbg << "[GMap]: Trying " << command << endl;
            db = getDataBufferFromProgram(command);
         }
         if ( db != NULL ) {
            retVal = internalLoad(*db);
         }
         delete db;
         delete [] command;
      }
   }

   
   uint32 endTime = TimeUtility::getCurrentMicroTime();
   if ( retVal ){
      mc2log << info << "Map 0x" << hex << getMapID() << dec << "(" 
             << getMapID() << ") "<< MC2CITE(m_name) << " loaded in " 
             << (endTime - startTime)/1000.0 << " ms" << endl; 
   }
   return retVal;
}

bool
OldGenericMapHeader::internalLoad(DataBuffer& dataBuffer)
{
   CLOCK_MAPLOAD(uint32 startTime = TimeUtility::getCurrentMicroTime());
   mc2dbg4 << "OldGenericMapHeader::internalLoad" << endl;
   // ***************************************************************
   //                                Read the general map information
   // ***************************************************************
   uint32 currentMapID = dataBuffer.readNextLong();
   // do check without map set bits
   if (currentMapID != MapBits::getMapIDWithoutMapSet(m_mapID)) {
      mc2log << fatal << "OldGenericMapHeader::createFromDataBuffer, currentMapID: " 
             << prettyMapIDFill(currentMapID) << " does not match filename, mapID: " 
             << prettyMapIDFill(m_mapID) << ". EXITING!" << endl;
      exit(1);
   }
   m_totalMapSize = dataBuffer.readNextLong(); // Not set!!!
   
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
   m_nativeLanguages = new Vector(nbrNativeLanguages);
   for (uint32 i=0; i<nbrNativeLanguages; i++) {
      m_nativeLanguages->addLast(dataBuffer.readNextLong());
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
         m_waspTime = 0;
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
         m_waspTime = 0;
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
         m_waspTime = 0;
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
      case (6) :  // OldGenericMap::m_userRightsTable
      case (7) :  // OldGenericMap::m_adminAreaCentres
      case (8) :  // OldGenericMap: lanes, new sign posts, categories and
                  //                cartographic item.
      case 9:     // MN3.5:
                  // GMSLane::m_laneType (long),
                  // GMSSignPostElement::m_elementClass (byte -> long)
                  // OldItem::m_nbrGroups (byte->long)
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
                 << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
                 << " ms, m_loadedVersion=" << int(m_loadedVersion) << endl;
                 startTime = TimeUtility::getCurrentMicroTime());
   return true;
}

bool
OldGenericMapHeader::internalSave(int outfile)
{
   uint32 startTime = TimeUtility::getCurrentMicroTime();

   // ***************************************************************
   //                                Save the general map information
   // ***************************************************************
   byte nbrNativeLanguages = 0;
   if (m_nativeLanguages != NULL) {
      nbrNativeLanguages = (byte) m_nativeLanguages->getSize();
   }
   
   byte nbrCurrencies = 0;
   if (m_currency != NULL)
      nbrCurrencies = (byte) m_currency->getSize();

   // version: 9
   byte version = 9;

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
   //    simpleItems(1)
   //    cartographicItems(1)
   m_nbrAllocators = uint16(numberInitialItemTypeAllocators)+9;

   // 256 for the copyright string
   DataBuffer* dataBuffer = new DataBuffer(20 + 4*nbrNativeLanguages + 
                                           4*nbrCurrencies +
                                           variableHeaderSize +
                                           4 + 8*m_nbrAllocators + 256);
   dataBuffer->fillWithZeros();

   dataBuffer->writeNextLong(m_mapID);
   dataBuffer->writeNextLong(0);    // Total size to be filled in later!!!

   // Save the current time
   m_creationTime = TimeUtility::getRealTime();
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
   for (uint32 i=0; i<nbrNativeLanguages; i++) {
      dataBuffer->writeNextLong(m_nativeLanguages->getElementAt(i));
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
   // NB: The copyright from map header is not used when running maps on 
   //     the server, if we used the copyright map supplier boxes xml file
   //     in the map generation.
   //     When adding copyright info from the copyright map supplier boxes
   //     xml file this header copyright is dis-regarded.
   const char* copyrightStringToSave = m_copyrightString;   
   if ( copyrightStringToSave == NULL ) {
      copyrightStringToSave = "© 2010";
   }
   
   dataBuffer->writeNextString( copyrightStringToSave );
   dataBuffer->alignToLongAndClear();

   mc2dbg << "[GenricMapHeader]: Header saved in "
          << (TimeUtility::getCurrentMicroTime()-startTime)/1000.0
          << " ms, version=" << int(version) << endl;
   bool retVal = ( write(outfile, 
                         dataBuffer->getBufferAddress(), 
                         dataBuffer->getCurrentOffset() ) ==
                   (int)dataBuffer->getCurrentOffset() );
   delete dataBuffer;
   
   return retVal;
}


void 
OldGenericMapHeader::setMapID(uint32 newMapID) 
{
   m_mapID = newMapID;
   char* tmpStr = new char[strlen(m_pathname)+1];
   strcpy(tmpStr, m_pathname);
   setFilename(tmpStr);
   delete [] tmpStr;
}

void 
OldGenericMapHeader::setCountryCode(StringTable::countryCode countryName)
{
   m_country = countryName;
}

void 
OldGenericMapHeader::setDrivingSide(bool driveRight)
{
   m_driveOnRightSide = driveRight;
}


void 
OldGenericMapHeader::addNativeLanguage(LangTypes::language_t lang)
{
   // If there is no nativeLanguages-array we create it here
   if (m_nativeLanguages == NULL)
      m_nativeLanguages = new Vector(2, 2);

   // Add the language to the array
   m_nativeLanguages->addLast(lang);
}

void 
OldGenericMapHeader::clearNativeLanguages()
{
   if (m_nativeLanguages != NULL)
      m_nativeLanguages->reset();

}

void 
OldGenericMapHeader::addCurrency(StringTable::stringCode cur )
{
   // If there is no currency-array we create it here
   if (m_currency == NULL)
      m_currency = new Vector(2, 2);

   // Add the currency to the array
   m_currency->addLast(cur);

}

void 
OldGenericMapHeader::clearCurrencies()
{
   if (m_currency != NULL)
      m_currency->reset();
}

void 
OldGenericMapHeader::setCopyrightString(const char* copyright)
{
   delete [] m_copyrightString;
   m_copyrightString = StringUtility::newStrDup(copyright);
}


void
OldGenericMapHeader::setFilename(const char* path)
{
   delete [] m_pathname;
   m_pathname = new char[strlen(path)+1];   // path + \0
   strcpy(m_pathname, path);

   delete [] m_filename; 
   m_filename = new char[strlen(path)+9+4+1];   // path + m_mapID + ".mcm" + \0
   sprintf(m_filename, "%s%09x%s", path, MapBits::getMapIDWithoutMapSet(m_mapID), ".mcm");

   mc2dbg4 << "Filename set to " << m_filename << endl;
}

// Replacement for code not compiling with gcc-3
inline static const char* time_str( uint32 time )
{
   time_t t( time );
   return asctime( localtime(&t) );
}


vector<MC2String> 
OldGenericMapHeader::getHeaderAsStrings() const
{
   vector <MC2String> strings;
   size_t tmpCharsSize = 4096;
   char tmpChars[tmpCharsSize];
   
   const char* trueStr = "true";
   const char* falseStr = "false";
   const char* value = NULL;

   // Map ID.
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_mapID: 0x%x = %u",
             m_mapID,
             m_mapID );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Map name
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_name: %s",
             m_name );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Loaded map version.
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_loadedVersion: %d",
             m_loadedVersion );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Total map size.
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_totalMapSize: 0x%x = %u", 
             m_totalMapSize, 
             m_totalMapSize );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));
   

   // Groups in location name order.
   value = falseStr;
   if ( m_groupsInLocationNameOrder ){
      value = trueStr;
   }
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_groupsInLocationNameOrder: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));



   // File name
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_filename: %s",
             m_filename );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Path name 
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_pathname: %s",
             m_pathname );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Map origin
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_origin: %s",
             m_origin );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Country code
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_country: 0x%x = %u",
             m_country,
             m_country );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   MC2String country = tmpChars;
   country.append(" (");
   country.append( StringTable::getString( 
                      StringTable::getCountryStringCode(m_country), 
                      (StringTable::languageCode)0 /*English*/ ) 
                   );
   country.append(")");
   strings.push_back(country);
   
   // Drive on right side
   value = falseStr;
   if ( m_driveOnRightSide ){
      value = trueStr;
   }
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_driveOnRightSide: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));
   
   
   // Native languages.
   MC2String nativeLangs("m_nativeLanguages(vector index:lang): ");
   if ( m_nativeLanguages != NULL ){
      
      for ( uint32 i=0; i<m_nativeLanguages->size(); i++){
         // Language index.
         snprintf( tmpChars, tmpCharsSize, "%d:", i );
         nativeLangs.append( tmpChars );
         
         // Language as string.
         value = LangTypes::getLanguageAsString( 
                    static_cast<LangTypes::language_t>
                    ( (*m_nativeLanguages)[i])  );
         nativeLangs.append( value );
         
         if ( ( m_nativeLanguages->size() - i ) > 1 ){
            // Write a comma to separate from coming langs.
            nativeLangs.append( ", ");
         }
      }
   }
   else{
      nativeLangs.append("NULL");
   }
   strings.push_back(nativeLangs);
   
   
   // Currencies
   MC2String currencies("m_currency(vector index:currency): ");
   if ( m_currency != NULL ){
      
      for ( uint32 i=0; i<getNbrCurrencies(); i++){
         // Language index.
         snprintf( tmpChars, tmpCharsSize, "%d:", i );
         currencies.append( tmpChars );
         
         // Language as string.
         value = 
            StringTable::getString( getCurrency(i),
                                    (StringTable::languageCode)0 /*eng*/ );
         currencies.append( value );
         
         if ( ( getNbrCurrencies() - i ) > 1 ){
            // Write a comma to separate from coming langs.
            currencies.append( ", ");
         }
      }
   }
   else{
      currencies.append("NULL");
   }
   strings.push_back(currencies);
   


   // Copyright string.
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_copyrightString: %s",
             m_copyrightString );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Number of allocators.
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_nbrAllocators: %u", 
             m_nbrAllocators );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Creation time
   value = time_str(m_creationTime);
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_creationTime: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // True creation time
   value = time_str( m_trueCreationTime );
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_trueCreationTime: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Wasp time
   value = time_str( m_waspTime );
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_waspTime: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Dynamic extradata time
   value = time_str( m_dynamicExtradataTime );
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_dynamicExtradataTime: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Utf8 strings
   value = falseStr;
   if ( m_utf8Strings ){
      value = trueStr;
   }
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_utf8Strings: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Map filtered
   value = falseStr;
   if ( m_mapFiltered ){
      value = trueStr;
   }
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_mapFiltered: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Map gfx data filtered
   value = falseStr;
   if ( m_mapGfxDataFiltered ){
      value = trueStr;
   }
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_mapGfxDataFiltered: %s",
             value );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));


   // Map country dir
   snprintf( tmpChars, 
             tmpCharsSize,
             "m_mapCountryDir: %s",
             m_mapCountryDir );
   tmpChars[tmpCharsSize-1] = '\x0'; // Make sure the string is terminated.
   strings.push_back(MC2String(tmpChars));
      

   //strings.push_back("NB! Not all values of the header are printed!");
   return strings;
} // getHeaderAsStrings


void OldGenericMapHeader::setTrueCreationTime( uint32 time ) {
   if ( time != MAX_UINT32 ) {
      m_trueCreationTime = time;
   } else {
      m_trueCreationTime = TimeUtility::getRealTime();
   }
}

void OldGenericMapHeader::setWaspTime( uint32 time ) {
   if (time != MAX_UINT32)
      m_waspTime = time;
   else
      m_waspTime = TimeUtility::getRealTime();
}

void OldGenericMapHeader::setDynamicExtradataTime( uint32 time ) {
   if ( time != MAX_UINT32 ) {
      m_dynamicExtradataTime = time;
   } else {
      m_dynamicExtradataTime = TimeUtility::getRealTime();
   }
}
