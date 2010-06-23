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

#include "MC2String.h"

#include "Types.h"

#include "Readable.h"


class TCPSocket;
class DataBuffer;
class MapSafeVector;
class GenericMap;

/**
 *   Readable for TCPSockets.
 */
class TCPSocketReadable : public Readable {
public:
   /**
    *   Creates a new TCPSocket readable.
    *   @param socket The socket. Will be owned by the TCPSocketReadable.
    */
   TCPSocketReadable(TCPSocket* socket);

   /**
    *   Destructor. Will destroy the socket.
    */
   virtual ~TCPSocketReadable();

   /**
    *   Tries to copy <code>size</code> bytes into <code>buffer</code>.
    *   @param buf The buffer to put the result into.
    *   @param size The numbeer of bytes to read.
    *   @return The number of bytes read or a negative number.
    */
   ssize_t read(byte* buffer, size_t size);

   /**
    *   Tries to copy <code>size</code> bytes into <code>buffer</code>.
    *   @param buf The buffer to put the result into.
    *   @param size The numbeer of bytes to read.
    *   @param timeout The timeout. May or may not be implemented.
    *   @return The number of bytes read or a negative number if failure
    *           or timeout.
    */
   ssize_t read(byte* buffer, size_t size, uint32 timeout);
   
protected:

   /**
    *   The socket of the readable.
    */
   TCPSocket* m_tcpSocket;
   
};

/**
 *   Readable that also writes everything it reads to a file.
 *   Mainly for use in RouteModule where the cached map is exactly
 *   the one sent from MapModule. This is to avoid too many changes
 *   in the RouteModule.
 */
class AutoWritingReadable : public Readable
{
public:
   /**
    *   Creates a new AutoWritingReadable.
    *   The header should be deleted by the caller.
    *   @param readable   Readable to read from and then write.
    *   @param filename   Final filename to put the cached data into.
    *   @param header     Header to write first in file.
    *   @param headerSize Size of header.
    */
   AutoWritingReadable(Readable* readable,
                       const char* filename,
                       const byte* header = NULL,
                       int headerSize = 0);
   
   /**
    *   Renames the temporary file to the right filename
    *   if all reads were succesful.
    *   Deletes the Readable.
    */
   ~AutoWritingReadable();

   /**
    *   Tries to copy <code>size</code> bytes into <code>buffer</code>.
    *   Also writes the data to a file if the reading was succesful.
    *   @param buf The buffer to put the result into.
    *   @param size The numbeer of bytes to read.
    *   @return The number of bytes read or a negative number.
    */
   ssize_t read(byte* buffer, size_t size);

   /**
    *   Tries to copy <code>size</code> bytes into <code>buffer</code>.
    *   @param buf The buffer to put the result into.
    *   @param size The numbeer of bytes to read.
    *   @param timeout The timeout. Not implemented, will wait forever.
    *   @return The number of bytes read or a negative number if failure
    *           or timeout.
    */
   ssize_t read(byte* buffer, size_t size, uint32 timeout);

private:
   /** The final filename */
   MC2String m_filename;

   /** The temporary file */
   int m_tmpDesc;

   /** The socket */
   Readable* m_readable;

   /** Current file to write to */
   FILE* m_writeFile;

   /** The name of the tempfile */
   MC2String m_tempName;
   
};

/**
 *   Readable that reads from file.
 */
class FileReadable : public Readable {
public:
   /**
    *   Creates a new DataBufferReadable.
    *   @param file The file to read from.
    */
   FileReadable(FILE* file);

   /**
    *   Closes the file.
    */
   virtual ~FileReadable();

   /**
    *   Tries to copy <code>size</code> bytes into <code>buffer</code>.
    *   @param buf The buffer to put the result into.
    *   @param size The numbeer of bytes to read.
    *   @return The number of bytes read or a negative number.
    */
   ssize_t read(byte* buffer, size_t size);

   /**
    *   Tries to copy <code>size</code> bytes into <code>buffer</code>.
    *   @param buf The buffer to put the result into.
    *   @param size The numbeer of bytes to read.
    *   @param timeout The timeout. May or may not be implemented.
    *   @return The number of bytes read or a negative number if failure
    *           or timeout.
    */
   ssize_t read(byte* buffer, size_t size, uint32 timeout);

protected:

   /// The file to read from.
   FILE* m_file;
   
};

/**
 *   Abstract class from which the maps in the modules should
 *   inherit in the future.
 */
class ModuleMap {

public:

   /**
    *    Uses the property value MODULE_CACHE_PATH to
    *    get the path where to put cached maps.
    *    Returns empty string if no caching.
    */
   static MC2String getCachePath();
   
   /**
    *    Creates a filename for a cachefile. 
    *    @param mapID The mapID of the map.
    *    @param loadMapRequestType Type of map.
    *    @param zoomLevel Zoomlevel, if wanted.
    */
   static MC2String getCacheFilename( uint32 mapID,
                                      uint32 loadMapRequestType,
                                      byte zoomlevel = 0);
   
   /**
    *    Gets a socket for loading maps from the MapModule. Should
    *    not be used by MapModule.
    *    @param mapID The ID of the map to load.
    *    @param loadMapRequestType @see MapRequestPacket::MapType.
    *    @param handshake String to send to the mapmodule when
    *                     connecting.
    *    @param zoomlevel The zoomlevel to request. Don't know if
    *                     any module uses it.
    *    @param mapVersion A pointer to the currently cached map version.
    *                      The newest available mapVersion will be returned.
    *    @param generatorVersion A pointer to the cached generator version.
    *                            Will be changed to the newest version if ok.
    *    @return A socket or NULL if the connection failed.
    */
   static TCPSocket* getMapLoadingSocket( uint32 mapID,
                                          uint32 loadMapRequestType,
                                          const char* handshake = "MapReq",
                                          byte zoomlevel = 0,
                                          MapSafeVector* loadedMaps = NULL,
                                          uint32* mapVersion = NULL,
                                          uint32* generatorVersion = NULL);


   /**
    *   Gets a readable to read the map from.
    *   Can be a socket or a file or a socketreader that writes
    *   to file.
    *   Same parameters as getMapLoadingSocket exept for the mapVersion
    *   and generatorVersion that are taken from cached file if that
    *   exists.
    */
   static Readable* getMapLoadingReadable( uint32 mapID,
                                           uint32 loadMapRequestType,
                                           const char* handshake = "MapReq",
                                           byte zoomlevel = 0,
                                           MapSafeVector* loadedMaps = NULL);

   /**
    *   Saves the cached map that is in the databuffer to the file
    *   where it will be looked for by the module with corresponding
    *   parameters.
    *   @param mapBuffer          DataBuffer containing the whole map.
    *   @param mapID              MapID of the map.
    *   @param loadMapRequestType Type of map. (From MapPacket).
    *   @param update             Not overwriting cache files if they were
    *                             created after corresponding map file.
    *   @param zoomLevel          Zoom level.
    *   @return True if map was saved OK.
    */
   static bool saveCachedMap(DataBuffer& mapBuffer,
                             const GenericMap* theMap,
                             uint32 loadMapRequestType,
                             uint32 mapVersion,
                             uint32 generatorVersion,
                             bool update,
                             byte zoomlevel = 0 );


   /**
    *   @return Returns true if the cache map file is up to data, i.e. saved
    *           after the corresponding map file.
    */
   static bool cacheIsUpToDate(uint32 mapID, 
                               MC2String mapFileName,
                               uint32 loadMapRequestType,
                               byte zoomlevel = 0 );


private:
   /**
    *    Creates a header for the cached file.
    *    The array should be deleted by the caller.
    */
   static byte* makeHeader(int& headerSize,
                           uint32 mapID,
                           uint32 mapVersion,
                           uint32 generatorVersion);
   
};

