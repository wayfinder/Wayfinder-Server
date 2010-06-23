/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Convert.h"

#include "M3Creator.h"
#include "OldGenericMap.h"
#include "GenericMap.h"
#include "Properties.h"
#include "MapModuleNoticeContainer.h"
#include "MapModuleNotice.h"
#include "STLStringUtility.h"
#include "PropertyHelper.h"
#include "File.h"
#include "DataBufferUtil.h"
#include "DataBuffer.h"
#include "TempFile.h"

#include <fstream>
#include <memory>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdexcept>

using namespace std;

void Convert::convertFile( const MC2String& srcFileName, 
                           const MC2String& destFileName,
                           const MapModuleNotice* mapNotice ) 
try {
   
   // we do not need to convert, 
   // a converted file already exist
   if ( File::fileExist( destFileName ) ) {
      mc2dbg << "[Convert]: file: " << destFileName 
             << " already exist, using it." << endl;
      return;
   }


   mc2dbg << "[Convert]: Converting file: " << srcFileName << " to " 
          << destFileName << endl;

   MC2String srcPath = STLStringUtility::dirname( srcFileName ) + "/";
   MC2String destPath = STLStringUtility::dirname( destFileName ) + "/";

   // Something must be wrong if the source and destination paths are
   // the same,
   mc2dbg << "[Convert]: srcPath = " << srcPath <<endl;
   mc2dbg << "[Convert]: destPath = " << destPath <<endl;
   
   if ( srcPath == destPath ) {
      throw MC2String("Source path and destination path can not be the same.");
   }

   // try to create directory
   struct stat fStat;
   if ( stat( destPath.c_str(), &fStat ) ) {   
      if ( File::mkdir_p( destPath ) == -1 && errno != EEXIST) {
         throw MC2String( "Can not create directory: " ) + destPath + "(" + 
            strerror(errno) + ")";
      }
   }

   // copy index.db file
   if ( ! File::fileExist( destPath + "index.db" ) ) {
      // copy index.db

      DataBuffer infile;

      if ( !infile.memMapFile( MC2String( srcPath + "index.db").c_str(), 
                               false,  // no readWrite
                               false ) ) { // no exit
         throw MC2String("Can not open index.db file: ") + srcPath + "index.db";
      }

      // move pointer to end of the buffer
      infile.readNextByteArray( infile.getBufferSize() );
      
      TempFile tempFile( "Convertsave", destPath,
                         "index.db", destPath );

      if ( ! tempFile.ok() ) {
         throw MC2String( "Can not create temp file. " ) + strerror( errno );
      }

      mc2dbg << "[GMH]: Writing to temp file " << tempFile.getTempFilename() << endl;

      // Copy file
      DataBufferUtil::saveBuffer( infile, tempFile.getFD() );
      
      // Chmod the file
      if ( fchmod( tempFile.getFD(), 
                   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ) == -1 ) {
         // This is not fatal, just anoying.
         mc2log << error << "[Convert]: Failed to chmod the temp file: " 
                << tempFile.getTempFilename() 
                << ". Error: " << strerror( errno ) << endl;
         
      }
   }

   // if the requested file is index.db (which we copied in previous section 8-) )
   // return successfully
   if ( STLStringUtility::basename( srcFileName ) == "index.db" ) {
      return;
   }

   auto_ptr<GenericMap> map;

   // create OldGenericMap which we then use with M3Creator
   // to convert to GenericMap
   
   uint32 creationTimeToCheck = MAX_UINT32;
   // scope oldMap, we dont need to have two maps loaded when we save
   auto_ptr<OldGenericMap> oldMap( OldGenericMap::
                                   createMap( srcFileName.c_str() ) );

   if ( oldMap.get() != NULL ) {
      creationTimeToCheck = oldMap->getCreationTime();
   }
      
   if ( mapNotice != NULL ) { 
      MC2_ASSERT( oldMap->getCreationTime() ==
                  mapNotice->getCreationTime() );
   }

   if ( oldMap.get() == NULL ) {
      throw MC2String("Failed to create OldGenericMap from file: ") 
         + srcFileName.c_str();
   }

   // convert from OldGenericMap to GenericMap

   M3Creator creator;
   creator.saveM3Map( *oldMap, destFileName );

} catch ( const FileUtils::FileException& fileError ) {
   throw MC2String( "Failed to convert. File problem. ") + fileError.what();
}

namespace {

MC2String getMapSetNumStr( uint32 mapSet )
{
   if ( mapSet == MAX_UINT32 ) {
      return "";
   } else {
      char tmpBuf[16];
      sprintf(tmpBuf, "%u", mapSet );
      return tmpBuf;
   }
}

MC2String getMapSetPropStr( uint32 mapSet )
{
   if ( mapSet == MAX_UINT32 ) {
      return "";
   } else {
      return MC2String("_") + getMapSetNumStr( mapSet );
   }
}

}

void Convert::getMapPaths( uint32 mapSet, 
                           MC2String& destPath,
                           MC2String& srcPath )
{


   MC2String resourceStr = "MAP_SERV_SOURCE_PATH";
   resourceStr += getMapSetPropStr( mapSet );
      

   try {

      srcPath = PropertyHelper::get<MC2String>( resourceStr.c_str() );

   } catch ( PropertyException err ) {

      // try default path if mapSet = 0
      MC2String mapPathNormal = PropertyHelper::get<MC2String>( "MAP_SERV_SOURCE_PATH" );
      if ( mapSet == 0 ) {
         srcPath = mapPathNormal;
      } else {
         throw err;
      }
   }


   srcPath += "/";

   destPath = PropertyHelper::get<MC2String>("MAP_SERV_DEST_PATH");

}

MC2String
Convert::getSourceFileName( const MC2String& srcPath,
                            uint32 mapID )
{
   char src_buff[255];
   sprintf(src_buff, "%09x", mapID);
   return STLStringUtility::addPathComponent( srcPath, src_buff );
}

MC2String
Convert::getDestFileName( const MC2String& dstPath,
                          uint32 mapID )
{
   char src_buff[255];
   sprintf(src_buff, "%09x", mapID);
   char buff[ 16 ];
   sprintf( buff, "-%d.m3", GenericMap::getMapVersion() );
   return STLStringUtility::addPathComponent( dstPath,
                                              MC2String(src_buff) + buff );
}


void Convert::convertFromIndex( uint32 mapSet, const vector<uint32>& mapIDs ) {

   MC2String mapPath, destPath;

   getMapPaths( mapSet, 
                destPath, mapPath );

   mc2dbg << "convertFromIndex " << mapSet << " destPath " << destPath
          << " mapPath " << mapPath << endl;

   MapModuleNoticeContainer noticeContainer;

   MC2String indexFilename = 
      STLStringUtility::addPathComponent( mapPath, "index.db" );

   mc2dbg << "[Convert]: loading " << indexFilename << endl;

   // MapSet should not be used here. It is set by the MapModule
   // when loading
   if ( ! noticeContainer.load( indexFilename.c_str(), MAX_UINT32 ) ) {
      throw MC2String( "[Convert]: Can not load file: " ) + indexFilename;
   }

   mc2dbg << "[Convert]: " << indexFilename << " loaded." << endl;


   for ( uint32 i = 0; i < noticeContainer.getSize(); ++i ) try {
      const MapModuleNotice *notice = noticeContainer.getElementAt( i );
      MC2_ASSERT( notice != NULL );

      MC2String srcFilename =
         getSourceFileName( mapPath, notice->getMapID() );
      
      MC2String destFilename =
         getDestFileName( /*STLStringUtility::addPathComponent( */destPath/*, mapPath )*/,
                          notice->getMapID() );

      if ( mapIDs.empty() || 
           std::find( mapIDs.begin(), mapIDs.end(), notice->getMapID() ) != mapIDs.end() ) {
         convertFile( srcFilename, destFilename, notice );
      }
   } catch ( PropertyException err ) {
      mc2log << error << "[Convert]: Property problem: " << err.what() << endl;
   } catch ( MC2String err ) {
      mc2dbg << "[Convert]: " << err << endl;
   }

}

void
Convert::convertSet( const std::vector<uint32>&  mapIDSet,
                     const vector<uint32>& mapIDs ) {
   uint32 maxMapID = PropertyHelper::get<uint32>("MAP_SET_COUNT");
   for ( uint32 i = 0; i < mapIDSet.size(); ++i ) {
      if ( mapIDSet[ i ] > maxMapID ) {
         char buff[10];
         sprintf( buff, "%d", mapIDSet[ i ] );
         MC2String error( MC2String("mapSet: ") + buff + " is out of range." );
         throw std::out_of_range( error.c_str() );
      }

      convertFromIndex( mapIDSet[ i ], mapIDs );
   }
}

void
Convert::convertAll()
{
   const char* mapSet = Properties::getProperty("MAP_SET_COUNT");
   uint32 nbrMapSet = Properties::getUint32Property("MAP_SET_COUNT", 1);
   vector<uint32> mapIDs;
   if ( mapSet == NULL ) {
      mc2dbg << "[MHAT]: No MAP_SET_COUNT - converting without mapSets"
             << endl;
      convertFromIndex( MAX_UINT32, mapIDs );
   } else {
      mc2dbg << "[MHAT]: converting all " << nbrMapSet 
             << " map sets." << endl;
      // convert from index
      for ( uint32 i = 0; i < nbrMapSet ; ++i ) {
         convertFromIndex( i, mapIDs );
      }
   }
}
