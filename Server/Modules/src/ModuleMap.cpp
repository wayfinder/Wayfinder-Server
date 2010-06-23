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
#include "ModuleMap.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "TCPSocket.h"
#include "DataBuffer.h"

#include "DatagramSocket.h"
#include "StringTable.h"
#include "multicast.h"
#include "MapPacket.h"
#include "SystemPackets.h"

#include "MapSafeVector.h"
#include "GenericMap.h"
#include "File.h" 
#include "FilePtr.h"
#include "NetUtility.h"
#include "ScopedArray.h"

#include "DataBufferCreator.h"

#define CACHED_MAP_HEADER_SIZE 48

// --- TCPSocketReadable

TCPSocketReadable::TCPSocketReadable(TCPSocket* tcpSocket)
{
   m_tcpSocket = tcpSocket;
}

TCPSocketReadable::~TCPSocketReadable()
{
   delete m_tcpSocket;
}

ssize_t 
TCPSocketReadable::read(byte* buffer, size_t size)
{
   return m_tcpSocket->readExactly(buffer, size);
}

ssize_t
TCPSocketReadable::read(byte* buffer, size_t size, uint32 timeout)
{
   return m_tcpSocket->readExactly(buffer, size, timeout);
}


// --- FileReadable

FileReadable::FileReadable(FILE* file)
{
   m_file = file;
}

FileReadable::~FileReadable()
{
   fclose(m_file);
}

ssize_t
FileReadable::read(byte* buffer, size_t size)
{
   return fread(buffer, 1, size, m_file);
}

ssize_t
FileReadable::read(byte* buffer, size_t size, uint32 timeout)
{
   return fread(buffer, 1, size, m_file);
}

// --- AutoWritingReadable
AutoWritingReadable::AutoWritingReadable(Readable* readable,
                                         const char* filename,
                                         const byte* header,
                                         int headerSize)
      :  m_filename(MC2String(filename)),
         m_readable(readable)
       
{
   // FIXME: Use same path as filename
   MC2String tmpDir = ModuleMap::getCachePath();
   char tempTemplate[1024];
   sprintf(tempTemplate, "%scachingXXXXXX", tmpDir.c_str());
   m_tmpDesc = mkstemp(tempTemplate);
   m_tempName = MC2String(tempTemplate);

   // Remove the old file if there is one
   mc2dbg << "[AWR]: Removing old file" << endl;
   int unl_res = unlink(filename);
   if ( unl_res == -1 ) {
      mc2dbg << "[ModuleMap]: Could not remove file since "
             << strerror(errno) << endl;
   }
   
   if ( m_tmpDesc < 0 ) {
      mc2dbg << "[AWR]: Could not make tempfile" << endl;
      m_writeFile = NULL;
   } else {
      m_writeFile = fdopen(m_tmpDesc, "w");
      if ( header ) {
         if ( fwrite(header, headerSize, 1, m_writeFile) != 1 ) {
            mc2dbg << "[AWR]: Could not write header: "
                   << strerror(errno)
                   << endl;
            fclose(m_writeFile);
            unlink(m_tempName.c_str());
            m_writeFile = NULL;
         }
      }
   }
}

AutoWritingReadable::~AutoWritingReadable()
{
   if ( m_writeFile != NULL ) {
      // Rename the file.
      rename(m_tempName.c_str(), m_filename.c_str());
      fclose(m_writeFile);
      // Seems like the fclose closes the fd too.
   }

   delete m_readable;
}

ssize_t 
AutoWritingReadable::read(byte* buffer, size_t size)
{
   if ( size == 0 ) {
      return 0;
   }
   // FIXME: What happens if the readable returns -1 more
   //        than once?
   ssize_t res = m_readable->read(buffer, size);
   if ( res >= 0 &&
        m_writeFile && fwrite(buffer, size, 1, m_writeFile) == 1 ) {
      // OK.
   } else {
      if ( m_writeFile ) {
         mc2dbg << "[AWR]: Could not read from socket or write to file: "
                << strerror(errno)
                << endl;      
         fclose(m_writeFile);
         // Don't write anymore to the file
         m_writeFile = NULL;
         // Delete the file
         unlink(m_tempName.c_str());
      } else if ( res <= 0 ) {
         mc2dbg << "[AWR]: Res == " << res << endl;
      }
   }
   return res;
}

ssize_t
AutoWritingReadable::read(byte* buffer, size_t size, uint32 timeout)
{
   return read(buffer, size);
}


MC2String
ModuleMap::getCachePath()
{
   const char* path = Properties::getProperty("MODULE_CACHE_PATH");
   if ( path == NULL || strlen(path) == 0 ) {
      return "";
   }

   // Check if the path ends with a slash
   if ( path[strlen(path)-1] == '/' ) {
      return path;
   } else {
      return MC2String(path)+"/";
   }
   
}

MC2String
ModuleMap::getCacheFilename( uint32 mapID,
                             uint32 loadMapRequestType,
                             byte zoomlevel )
{
   char fileName[1024];

   MC2String path = getCachePath();
   if ( path.length() == 0 ) {
      return "";
   }
   
   sprintf(fileName, "%s%09x_z%04u_%04u%s",
           path.c_str(),
           mapID,
           zoomlevel,
           loadMapRequestType,
           ".cached");
   return MC2String(fileName);
}

TCPSocket*
ModuleMap::getMapLoadingSocket( uint32 mapID,
                                uint32 loadMapRequestType,
                                const char* handshake,
                                byte zoomlevel,
                                MapSafeVector* loadedMaps,
                                uint32* mapVersion,
                                uint32* generatorVersion )
{
   uint32 mapVersionToUse = MAX_UINT32;
   uint32 generatorVersionToUse = MAX_UINT32;
   if ( mapVersion != NULL && generatorVersion != NULL ) {
      // Set the versions to use to the ones sent in
      mapVersionToUse = *mapVersion;
      generatorVersionToUse = *generatorVersion;
      // Set the ones sent in to MAX_UINT32 to detect errors
      *mapVersion = MAX_UINT32;
      *generatorVersion = MAX_UINT32;
   }
   
   DatagramReceiver receiver(8000, DatagramReceiver::FINDFREEPORT);
   
   mc2dbg4 << here << " localport " << receiver.getPort() << endl;

   uint32 mapip = MultiCastProperties::getNumericIP( MODULE_TYPE_MAP, true );
   uint16 mapport = MultiCastProperties::getPort( MODULE_TYPE_MAP, true );

   // check map set
   uint32 mapSet = Properties::getMapSet();
   if (mapSet != MAX_UINT32) {
      // code also exists in PacketContainer.cpp, move to utility function?
      mc2dbg4 << "[ModuleMap] going to change IP and port due to mapSet being set. "
                "mapSet: " << mapSet << ", IP before: " << prettyPrintIP(mapip)
             << ", port before: " << mapport << endl;
      mapip = mapip + (mapSet << 8);
      mapport = mapport | (mapSet << 13);
      mc2dbg4 << "[ModuleMap] changed IP and port. IP now: " << prettyPrintIP(mapip)
             << ", port now: " << mapport << endl;
   }
   
   uint32 status = StringTable::NOT;
   int maxRetries = 10;
   int nbrRetries = 0;

   Packet _pack(65536); // For receiving the mapreply
   DatagramSender sock;

   const int originalwaittime    =  2500000; // Wait 2.5 seconds.
   const int mapnotfoundwaittime =  2500000; // Wait 2.5 seconds.
   int waittime = originalwaittime;

   TCPSocket* TCPsock = NULL;
   
   while ( status != StringTable::OK && nbrRetries++ <= maxRetries ) {
      if ( nbrRetries > 1 ) {
         mc2log << info << here << " Retrying " << nbrRetries - 1 
                << " of " << maxRetries << endl;
      }
      if(loadedMaps != NULL)
         loadedMaps->jobThreadIsAlive(); // reset Jthread timeout clock.
      MapRequestPacket reqpack( uint16(1),  // reqID
                                uint16(1),  // PacketID
                                byte(loadMapRequestType), // Type of module
                                mapID,      // mapID
                                zoomlevel,  // Well, zoomlevel.
                                mapVersionToUse,
                                generatorVersionToUse
                                ); 
      reqpack.setOriginIP( NetUtility::getLocalIP() );
      reqpack.setOriginPort( receiver.getPort() );
      reqpack.setResendNbr((byte) nbrRetries-1);
      
      // Send request to open TCP connection between local and mapmodule
      
      if (!(sock.send(&reqpack, mapip, mapport))) {
         mc2log << error << here << " could not send - retrying"
                << endl;
         continue; // Go another round in the loop.
      }
      mc2dbg4 << "After send!" << endl;
      
      // Receive packet with ip and port to a mapModule
      if (!(receiver.receive(&_pack, waittime))) {
         mc2log << error << here << " error receiving ack - retrying"
                << endl;
         waittime = originalwaittime;
         continue; // Go another round in the loop.
      }

      bool timeout = false;
      while ( _pack.getSubType() == Packet::PACKETTYPE_ACKNOWLEDGE &&
              ! timeout ) {
         AcknowledgeRequestReplyPacket* ackPack =
            static_cast<AcknowledgeRequestReplyPacket*>(&_pack);
         uint32 packtime =
            ((AcknowledgeRequestReplyPacket*)&_pack)->getETA() * 1000;
         mc2log << info << "Got ack with " << packtime << " us delay" << endl;
         if ( ackPack->getStatus() != StringTable::OK ) {
            return NULL;
         }
         timeout = !receiver.receive(&_pack, packtime);         
      }

      if ( timeout ) {
         mc2log << error << "Got timeout after receiving ack-pack" << endl;
         continue; // Go around again
      }

      if ( _pack.getSubType() != Packet::PACKETTYPE_MAPREPLY ) {
         mc2log << error << "Got packet with subtype "
                << _pack.getSubTypeAsString()
                << " when expecting loadmapreply" << endl;
         continue; // Please try again.
      }
      
      MapReplyPacket* pack = (MapReplyPacket *)&_pack;
      
      status = pack->getStatus();
      
      if ( status != StringTable::OK || pack->getMapID() != mapID ) {
         mc2log << warn << "Got status \""
                << StringTable::getString(
                   StringTable::stringCode(pack->getStatus()),
                   StringTable::ENGLISH)
                << "\"-retrying. Wanted map = "
                << mapID
                << ", got reply for map = " << pack->getMapID() << endl;
         if ( status == StringTable::MAPNOTFOUND ) {
            // Wait longer if map not found ( loading? )
            waittime = mapnotfoundwaittime; 
         }
         if ( mapID != pack->getMapID() ) {
            status = StringTable::NOT; // Keep the loop running.
         }
         continue; // Go another round in the loop.
      }

      if ( mapVersion != NULL && generatorVersion != NULL ) {
         // Set the versions so that we know what we are caching.
         mc2dbg8 << "[ModuleMap]: Version from packet "
                 << hex << pack->getMapVersion() << ":"
                 << pack->getGeneratorVersion() << dec << endl;
         *mapVersion       = pack->getMapVersion();
         *generatorVersion = pack->getGeneratorVersion();

         // Check if new map is needed.
         if ( ! pack->newMapNeeded(mapVersionToUse,
                                   generatorVersionToUse) ) {
            // Same version. Use the cached map.
            return NULL;
         } 
      }
      
      TCPsock = new TCPSocket;      
      mc2dbg4 << here << " opening socket" << endl;
      TCPsock->open();
      
      uint32 ip = pack->getReplyIP();
      uint16 port = pack->getReplyPort();
      
      if ( ! TCPsock->connect(ip, port ) ) {
         mc2log << error << "Couldn't connect to " << ip
                << ":" << port << " - retrying" << endl;
         // Set status to not ok.
         status = StringTable::NOT;
         TCPsock->close();
         delete TCPsock;
         TCPsock = NULL;                  
         continue; // Please try again.
      }
      
      // Handshaking
      mc2dbg4 << "Starting handshake" << endl;
      int length = strlen(handshake) + 1;
      if ( TCPsock->writeAll( (byte*)handshake, length ) != length ) {
         mc2log << error << here << " handshake failed " << endl;
         status = StringTable::NOT;
         TCPsock->close();
         delete TCPsock;
         TCPsock = NULL;
         continue; // Please try again.
      } else {
         mc2dbg4 << "done" << endl;
      }
   }
   return TCPsock; // Should be NULL if we failed
}

byte*
ModuleMap::makeHeader(int& headerSize,
                      uint32 mapID,
                      uint32 mapVersion,
                      uint32 generatorVersion)
{
   headerSize = CACHED_MAP_HEADER_SIZE;
   byte* header = new byte[headerSize];
   // Zero the header
   memset(header, 0, headerSize);
   DataBuffer dataBuff(header, headerSize);
   
   dataBuff.writeNextLong(0);
   dataBuff.writeNextLong(mapID);
   dataBuff.writeNextLong(mapVersion);
   dataBuff.writeNextLong(generatorVersion);
   return header;
}

Readable*
ModuleMap::getMapLoadingReadable(uint32 mapID,
                                 uint32 loadMapRequestType,
                                 const char* handshake,
                                 byte zoomlevel,
                                 MapSafeVector* loadedMaps )
{
   FileUtils::FilePtr file;

   MC2String cacheFileName = getCacheFilename(mapID,
                                           loadMapRequestType,
                                           zoomlevel);
   
   mc2dbg << "[ModuleMap]: Filename would be \""
          << cacheFileName << '"' << endl ;

   bool useCache = cacheFileName.length();
   
   if ( useCache ) {
      // If the cache filename exists we should try the cache.
      file.reset( fopen( cacheFileName.c_str(), "r" ) );
   }

   TCPSocket* tcpsock = NULL;
   uint32 mapVersion           = MAX_UINT32;
   uint32 generatorVersion     = MAX_UINT32;
   
   if ( file.get() != NULL ) {
      mc2dbg << "[ModuleMap]: Cached map found" << endl;

      // Check versions of map in file.
      ScopedArray<byte> headerBuf( new byte[CACHED_MAP_HEADER_SIZE] );
      
      uint32 fileMapVersion       = MAX_UINT32;
      uint32 fileGeneratorVersion = MAX_UINT32;
      uint32 fileMapID            = MAX_UINT32;
      
      if ( fread( headerBuf.get(), CACHED_MAP_HEADER_SIZE, 1, file.get() ) == 1 ) {
         DataBuffer dataBuf(headerBuf.get(), CACHED_MAP_HEADER_SIZE);
         uint32 first         = dataBuf.readNextLong();
         first = first;
         fileMapID            = dataBuf.readNextLong();
         fileMapVersion       = dataBuf.readNextLong();
         fileGeneratorVersion = dataBuf.readNextLong();
         if ( fileMapID == mapID ) {
            mapVersion = fileMapVersion;
            generatorVersion = fileGeneratorVersion;
         } else {
            mc2dbg << "[ModuleMap]: Wrong mapid in file - was "
                   << fileMapID << " should be " << mapID
                   << endl;
         }
      } 
      headerBuf.reset( NULL );

      mc2dbg << "[ModuleMap]: Version from file 0x"
             << hex << fileMapVersion << ":" << dec << fileGeneratorVersion
             << endl;
      
      tcpsock = getMapLoadingSocket(mapID,
                                    loadMapRequestType,
                                    handshake,
                                    zoomlevel,
                                    loadedMaps,
                                    &mapVersion,
                                    &generatorVersion);

      mc2dbg << "[ModuleMap]: Version from getMapLoadingSocket "
             << hex << "0x" << mapVersion << dec << ":" << generatorVersion
             << endl;
      
      if ( (tcpsock == NULL ) &&
           ( mapVersion == fileMapVersion ) &&
           ( generatorVersion == fileGeneratorVersion ) ) {
         mc2dbg << "[ModuleMap]: Cached file OK" << endl;
         return new FileReadable(file.release());
      } else {
         if ( tcpsock != NULL ) {
            mc2dbg << "[ModuleMap]: Versions differ (file/MM)." << endl;
         } else {
            mc2dbg << "[ModuleMap]: Wrong version or no socket." << endl;
         }
      }
   } else {
      mc2dbg << "[ModuleMap]: No cache file" << endl;
      // Just get the socket and load.
      tcpsock = getMapLoadingSocket(mapID,
                                    loadMapRequestType,
                                    handshake,
                                    zoomlevel,
                                    loadedMaps,
                                    &mapVersion,
                                    &generatorVersion);
   }
   
   if ( tcpsock != NULL && useCache ) {
      mc2dbg << "[ModuleMap]: Will write to cache" << endl;
      // We do not have the map cached or it may have been the wrong
      // version.
      int headerSize = 0;
      ScopedArray<byte> header( makeHeader( headerSize,
                                            mapID, mapVersion, 
                                            generatorVersion) );
               
      TCPSocketReadable* tsockRead = new TCPSocketReadable( tcpsock );
      
      return new AutoWritingReadable( tsockRead,
                                      cacheFileName.c_str(),
                                      header.get(),
                                      headerSize );
   } else if ( tcpsock ) {
      mc2dbg << "[ModuleMap]: Will not write to cache" << endl;
      return new TCPSocketReadable(tcpsock);
   } else {
      mc2dbg << "[ModuleMap]: No socket" << endl;
      // Loading failed
      return NULL;
   }
}


bool 
ModuleMap::cacheIsUpToDate(uint32 mapID, MC2String mapFileName,
                           uint32 loadMapRequestType,
                           byte zoomlevel){
   MC2String finalFileName( ModuleMap::getCacheFilename(mapID,
                                                        loadMapRequestType, 
                                                        zoomlevel) );
   if ( File::fileExist( finalFileName )){
      struct stat cacheInfo;
      if (stat(finalFileName.c_str(), &cacheInfo) != 0){
         mc2log << error << "Could not read file times for: " 
                << finalFileName << endl;
         MC2_ASSERT(false);
      }
      time_t cacheModTime = cacheInfo.st_mtime;
 
      struct stat mapInfo;
      MC2String realMapFileName = 
         DataBufferCreator::getMapOrIdxPath(mapFileName);
      if (stat(realMapFileName.c_str(), &mapInfo) != 0){
         mc2log << error << "Could not read file time for: " 
                << realMapFileName << endl;
         MC2_ASSERT(false);
      }
      time_t mapModTime = mapInfo.st_mtime;
 
      mc2dbg << "[ModuleMap]: Comparing file times for " << finalFileName 
             << " and " << realMapFileName << endl;
         
      if ( cacheModTime > mapModTime ){
         mc2dbg << info << "[ModuleMap]: Cache file " << finalFileName 
                << " is up to date, not saving." << endl;
         // This cache file is up to date.
         return true;
      }
   }
   return false;
}


bool
ModuleMap::saveCachedMap(DataBuffer& mapBuffer,
                         const GenericMap* theMap,
                         uint32 loadMapRequestType,
                         uint32 mapVersion,
                         uint32 generatorVersion,
                         bool update,
                         byte zoomlevel)
{
   uint32 mapID = theMap->getMapID();
   MC2String finalFileName(
      ModuleMap::getCacheFilename(mapID,
                                  loadMapRequestType, zoomlevel) );
   if ( update && cacheIsUpToDate(theMap->getMapID(), theMap->getFilename(), 
                                  loadMapRequestType, zoomlevel) ){
      return true;
   }
   
   const char* fileName = finalFileName.c_str();
   MC2String tmpDir = ModuleMap::getCachePath();
   if ( tmpDir[0] == '\0' ) {
      mc2log << info << "[ModuleMap]: No cachepath set - will not save "
             << "map 0x" << hex << mapID << dec << endl;
      return false;
   } else {
      mc2log << info << "[ModuleMap]: Will try to save "
             << MC2CITE(fileName) << endl;
   }
    
   char tempTemplate[1024];
   sprintf(tempTemplate, "%scachingXXXXXX", tmpDir.c_str());
   int tmpDesc = mkstemp(tempTemplate);
   MC2String tempName = MC2String(tempTemplate);
   // Remove the old file if there is one ( to make space for new map )
   mc2dbg << "[ModuleMap]: Removing " << MC2CITE(fileName)
          << " if it exists" << endl;
   unlink(fileName);

   int headerSize;
   byte* header = makeHeader( headerSize, mapID, mapVersion, generatorVersion);
   
   FILE* writeFile = NULL;
   if ( tmpDesc < 0 ) {
      mc2dbg << "[ModuleMap]: Could not make tempfile" << endl;
      writeFile = NULL;
      return false;
   } else {
      writeFile = fdopen(tmpDesc, "w");
      if ( header ) {
         if ( fwrite(header, headerSize, 1, writeFile) != 1 ) {
            mc2dbg << "[ModuleMap]: Could not write header: "
                   << strerror(errno)
                   << endl;
            fclose(writeFile);
            unlink(tempName.c_str());
            writeFile = NULL;
            return false;
         }
      } else {
         mc2log << "[ModuleMap]: Header is NULL" << endl;
         return false;
      }
   }
   delete [] header;
   
   MC2_ASSERT( writeFile != NULL );
   // Now the header should be written.
   if ( fwrite(mapBuffer.getBufferAddress(),
               mapBuffer.getBufferSize(), 1, writeFile) != 1 ) {
      fclose(writeFile);
      unlink(tempName.c_str());
      writeFile = NULL;
      mc2dbg << "[ModuleMap]: Could not write map data" << endl;
      return false;
   } else {
      rename(tempName.c_str(), fileName);
      chmod (fileName, (S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) );
      fclose(writeFile);
      mc2log << info << "[ModuleMap]: Map 0x" << hex << mapID << dec 
             << " saved." << endl;
      return true;
   }
}
