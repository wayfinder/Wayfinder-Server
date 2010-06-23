/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAP_CLIENTPRECACHESFD_QUERTY_H
#define TILEMAP_CLIENTPRECACHESFD_QUERTY_H

#include "TileMapQuery.h"
#include "TileMapClientPrecacheQuery.h"
#include "LangTypes.h"
#include "MC2SimpleString.h"

#include <vector>

class SharedBuffer;
class DBufRequester;
class ServerTileMapFormatDesc;
class SFDSavableHeader;
class TileMapParams;
class TileCollectionNotice;
class MC2BoundingBox;

struct TileMapClientSFDQueryPriv;

/**
 *   Class to use when precaching tilemaps for client use.
 */
class TileMapClientSFDQuery : public TileMapClientPrecacheQuery {
public:

   TileMapClientSFDQuery( const ServerTileMapFormatDesc* mapDesc, 
                          const MC2SimpleString& name,
                          bool debug,
                          const MC2BoundingBox& bbox,
                          const set<int>& layers,
                          const set<MC2SimpleString>& extraParams,
                          bool useGzip,
                          LangTypes::language_t lang,
                          uint32 scale );

   virtual ~TileMapClientSFDQuery();

   /**
    *   Returns the result buffer.
    */
   const SharedBuffer* getResult() const;
   
   int getNextParams( vector<MC2SimpleString>& params,
                      int maxNbr );

   bool isDone() const;

   int addBuffers( const bufVect_t& buffs );
protected:

   /// Used by constructor to create the cache.
   DBufRequester& createCache();

   /**
    *   @return the number of params done so far.
    */
   virtual uint32 getNbrDone() const;

   /**
    *   @return the number of still wanted tiles.
    */
   virtual uint32 getNbrStillWanted() const;

private:

   /**
    *   Creates the binary search buffer and updates the header with more
    *   information.
    */
   void createBinarySearchBufferAndUpdateHeader( SFDSavableHeader& header,
                                                 const paramSet_t& params );
   void createHeader();
   void appendBuffers( const bufVect_t& buffers );
   /**
    * writes data and string buffer to file, will destroy both buffers
    * @param dataBuffer buffer with "G" values
    * @param stringBuffer buffer with "T" values
    */
   void appendBuffers( bufVect_t& dataBuffer, 
                       bufVect_t& stringBuffer );

   /**
    *   Creates the buffer to send to the client.
    */
   const SharedBuffer* createBuffer();

   template <typename T>
   const SharedBuffer* createBuffer( TileCollectionNotice& collection,
                                     const paramSet_t& otherParams,
                                     T paramsBegin, T paramsEnd );

   /**
    *   The cache that is used during the life of the
    *   requester.
    */
   DBufRequester* m_realCache;
   
   /// Contains the bytes to send
   vector<uint8> m_storage;
   /// Buffer to return, points into m_storage
   SharedBuffer* m_resultBuf;

   /// The name of the cache (utf8).
   MC2SimpleString m_name;

   TileMapClientSFDQueryPriv* m_priv;

};

#endif
