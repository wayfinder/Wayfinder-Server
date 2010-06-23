/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFDHEADER_H
#define SFDHEADER_H 

#include "config.h"

#include "LangTypes.h"

#include <vector>
#include "MC2SimpleString.h"

class RouteID;
class TileCollectionNotice;
struct impRange_t;
class TileMapParams;

/**
 *    Header to a SFD cache.
 */
class SFDHeader {
public:
   /**
    *   Creates an empty SFDHeader.
    */
   SFDHeader();
   
   /// Deletes the requester
   virtual ~SFDHeader();

   /**
    *    Returns the size of one string offset entry.
    */
   uint32 getStrIdxEntrySizeBits() const; 

   /**
    *    Returns the number of strings. 
    */
   uint32 getNbrStrings() const; 

   /**
    *    Returns the start of the string index.
    */
   uint32 getStrIdxStartOffset() const;

   /**
    *    Returns the offset in the file where
    *    the string data starts.
    */
   uint32 getStrDataStartOffset() const;

   /**
    *    Returns the start offset for the buffer
    *    size index.
    */
   uint32 getBufferIdxStartOffset() const {
      return m_bufferIdxStartOffset;
   }

   /**
    *    Returns the offset for where the
    *    buffers start.
    */
   uint32 getBufferDataStartOffset() const {
      return m_bufferDataStartOffset;
   }

   /// True if the desc may be in the cache. False if definitely not.
   bool maybeInCache( const MC2SimpleString& desc ) const;

   /**
    *    Returns the size of the longest string in the file.
    */
   uint32 maxStringSize() const;

   /**
    *    Returns if strings are null terminated in the file.
    */
   bool stringsAreNullTerminated() const;

   /**
    *    Returns the offset for where the offset to the MultiBuffer data 
    *    for the specified param can be read. 
    *    -1 if the param is not included in this cache.
    */
   int getMultiBufferOffsetOffset( const TileMapParams& param ) const;
   
   ///   Get the name (utf8) of the cache.
   const MC2SimpleString& getName() const;

   ///   If the param strings is present for debug reasons in the file
   bool readDebugParams() const;
  
   ///   Get the creation time in UTC, seconds since 00:00:00 1 January 1970.
   uint32 getCreationTime() const;

   ///   Get the version of the sfd.
   uint32 getVersion() const;

   /**
    * Get the initial characters of the params in the StrBuff.
    */
   const vector<char>& getInitialCharacters() const;

   /**
    * Get the routeIDs in this header.
    */
   const vector<RouteID>& getRouteIDs() const;

   /**
    * Get reference to tile collection.
    */
   const vector<TileCollectionNotice>& getTileCollection() const;

   /**
    *    Get the importance range for the specified param.
    *    NULL if not found.
    */
   const impRange_t* getImportanceRange( const TileMapParams& param ) const; 

protected:

   /// Need to access the variables.
   friend class TileMapClientSFDQuery;
     
   /// The size of one string offset entry.
   uint32 m_strIdxEntrySizeBits;

   /// Number of strings.
   uint32 m_nbrStrings;
   
   /// The start of the string index.
   uint32 m_strIdxStartOffset;

   /// The offset in the file where the string data starts.
   uint32 m_strDataStartOffset;
  
   // Position for the start of the buffer index.
   uint32 m_bufferIdxStartOffset;

   // Position for the start of the buffer data.
   uint32 m_bufferDataStartOffset;
 
   /// The different tile collections.
   vector<TileCollectionNotice> m_tileCollection;
   
   /// The version.
   int m_version;

   /// Total file size of the cache.
   int m_fileSize;

   /// Size of the header.
   int m_headerSize;

   /// The length of the largest string.
   int m_maxStringSize;
   
   /// List of the intial characters of the strings present in the cache.
   vector<char> m_initialCharacters;

   /// The route ids in the cache.
   vector<RouteID> m_routeIDs;

   /// If the strings are null terminated.
   bool m_stringsAreNullTerminated;
   
   /// If the cache header has been initialized.
   bool m_initialized;
 
   /// True if we are shutting down
   bool m_shuttingDown;
   
   /// Creation time (UTC, seconds since 00:00:00 1 January 1970).
   uint32 m_creationTime;
   
   /// The name of this cache (utf8).
   MC2SimpleString m_name;

   /// If the param strings is present for debug reasons in the file
   bool m_readDebugParams;

};

// --- Inlines ---

inline const vector<char>&
SFDHeader::getInitialCharacters() const {
   return m_initialCharacters;
}

inline const vector<RouteID>&
SFDHeader::getRouteIDs() const {
   return m_routeIDs;
}

inline const vector<TileCollectionNotice>& 
SFDHeader::getTileCollection() const {
   return m_tileCollection;
}

#endif
