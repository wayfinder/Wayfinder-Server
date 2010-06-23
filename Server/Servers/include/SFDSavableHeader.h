/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SFDHEADERSAVABLE_H
#define SFDHEADERSAVABLE_H 

#include "config.h"

#include "SFDHeader.h"

class SharedBuffer;

/**
 *    SFDHeader that can be saved to buffer.
 */
class SFDSavableHeader : public SFDHeader
{
public:
   friend class TileMapClientSFDQuery;

   /**
    *    Create the header with a name.
    */ 
   SFDSavableHeader( const MC2SimpleString& name, bool debug );
  
   /**
    *    Destructor.
    */
   virtual ~SFDSavableHeader();
  
   /**
    *    Save into a buffer. Will also update some members.
    */
   int save( SharedBuffer& buf );

   /**
    *    Will update the meta data members in this class, given the
    *    parameter string.
    */
   void updateMetaData( const MC2SimpleString& string );

   /**
    * Get the multiBufferOffsetStartOffset.
    */
   uint32 getMultiBufferOffsetStartOffset() const;

   /**
    * Sets the name of the header.
    */
   void setName( const MC2SimpleString& name );

   /**
    * Updates the multiBufferOffsetStartOffset, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setMultiBufferOffsetStartOffset( uint32 offset );

   /**
    * Updates the multiBufferStartOffset, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setMultiBufferStartOffset( uint32 offset );

   /**
    * Updates the bufferDataStartOffset, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setBufferDataStartOffset( uint32 offset );

   /**
    * Updates the value, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setStringsAreNullTerminated( bool val );

   /**
    * Updates the value, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setStrIdxStartOffset( uint32 offset );

   /**
    * Updates the value, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setNbrStrings( uint32 nbr );

   /**
    * Updates the value, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setStrDataStartOffset( uint32 offset );

   /**
    * Updates the value, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setBufferIdxStartOffset( uint32 offset );

   /**
    * Updates the value, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setMaxStringSize( uint32 size );

   /**
    * Updates the value, warning do not use!
    * Use only if you are working with TileMapClientSFDQuery or replacement
    * for it.
    */
   void setVersion( uint32 v );

   /**
    * Writes new file size in buff.
    */
   void writeFileSize( uint32 fileSize, SharedBuffer& buf );

private:

   /// Start offset for the multibuffer offsets.
   uint32 m_multiBufferOffsetStartOffset;

   /// Start offset for the multibuffers.
   uint32 m_multiBufferStartOffset;
   
};


// --- Inlines ---

inline void
SFDSavableHeader::setName( const MC2SimpleString& name ) {
   m_name = name;
}

inline uint32
SFDSavableHeader::getMultiBufferOffsetStartOffset() const {
   return m_multiBufferOffsetStartOffset;
}

inline void
SFDSavableHeader::setMultiBufferOffsetStartOffset( uint32 offset ) {
   m_multiBufferOffsetStartOffset = offset;
}

inline void
SFDSavableHeader::setMultiBufferStartOffset( uint32 offset ) {
   m_multiBufferStartOffset = offset;
}

inline void
SFDSavableHeader::setBufferDataStartOffset( uint32 offset ) {
   m_bufferDataStartOffset = offset;
}


inline void
SFDSavableHeader::setStringsAreNullTerminated( bool val ) {
   m_stringsAreNullTerminated = val;
}

inline void
SFDSavableHeader::setStrIdxStartOffset( uint32 offset ) {
   m_strIdxStartOffset = offset;
}

inline void
SFDSavableHeader::setNbrStrings( uint32 nbr ) {
   m_nbrStrings = nbr;
}

inline void
SFDSavableHeader::setStrDataStartOffset( uint32 offset ) {
   m_strDataStartOffset = offset;
}

inline void
SFDSavableHeader::setBufferIdxStartOffset( uint32 offset ) {
   m_bufferIdxStartOffset = offset;
}

inline void
SFDSavableHeader::setMaxStringSize( uint32 size ) {
   m_maxStringSize = size;
}

inline void
SFDSavableHeader::setVersion( uint32 v ) {
   m_version = v;
}


#endif
