/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NPARAM_H
#define NPARAM_H

#include "config.h"
#include "MC2Coordinate.h"
#include "MC2String.h"
#include <vector>
#include <netinet/in.h>
#include <iomanip>
#include "UTF8Util.h"

/**
 * Class holding a NavServerProt, v10+, param.
 * All complex values (16 bit, 32 bit) are considered to have
 * the most significant byte first. All data is in network
 * order that is.
 *
 */
class NParam {
   public:
      /**
       * The types of NParam content, not used directly.
       */
      enum NParam_t {
         /**
          * The length may be 0 or 1. If 0 length then value is true. 
          * If length is 1 then the value is false if the byte is 0
          * true if not.
          */
         Bool,
         /// Length of 1, values 0-255 are possible. 
         Byte,
         /// Length of 2, values 0-65535 are possible.
         Uint16,
         /// Length of 4, values 0-4294967295 are possible.
         Uint32,
         /// Length of 4, values -2147483648 to 2147483647 are possible.
         Int32,
         /// Array of bytes ended by an null-byte, integer value of 0.
         /// Strings are utf-8 encoded.
         String,
         /// An array of length bytes.
         Byte_array,
         /// An array of length/2 uint16s.
         Uint16_array,
         /// An array of length/4 uint32s.
         Uint32_array,
         /// An array of length/4 int32s.
         Int32_array,
      };


      /**
       * Constructor.
       * 
       * @param paramID The parameter ID.
       * @param buff The buffer with the parameter data.
       * @param len The length of the parameter data.
       */
      NParam( uint16 paramID, const byte* buff, uint16 len );


      /**
       * Constructor from bool.
       */
      explicit NParam( uint16 paramID, bool data );


      /**
       * Constructor from byte.
       */
      NParam( uint16 paramID, byte data );


      /**
       * Constructor from uint16.
       */
      NParam( uint16 paramID, uint16 data );


      /**
       * Constructor from uint32.
       */
      NParam( uint16 paramID, uint32 data );


      /**
       * Constructor from int32.
       */
      NParam( uint16 paramID, int32 data );


      /**
       * Constructor from string.
       */
      NParam( uint16 paramID, const char* data, bool usesLatin1 = false );


      /**
       * Constructor from MC2String.
       */
      NParam( uint16 paramID, const MC2String& data, bool usesLatin1 = false );


      /**
       * Constructor from uint16 array.
       */
      NParam( uint16 paramID, const uint16* data, uint16 len );


      /**
       * Constructor from uint32 array.
       */
      NParam( uint16 paramID, const uint32* data, uint16 len );


      /**
       * Constructor from int32 array.
       */
      NParam( uint16 paramID, const int32* data, uint16 len );


      /**
       * Constructor from MC2Coordinate.
       */
      NParam( uint16 paramID, MC2Coordinate data );


      /**
       * Constructor for param without data.
       */
      NParam( uint16 paramID );


      /**
       * Get the paramID.
       */
      uint16 getParamID() const;


      /**
       * Set the paramID.
       */
      void setParamID( uint16 paramID );


      /**
       * Get the buffer length.
       */
      uint16 getLength() const;


      /**
       * Get const referense to buffer.
       */
      const byte* getBuff() const;


      /**
       * Get buffer as a bool.
       */
      bool getBool() const;


      /**
       * Get buffer as a byte.
       */
      byte getByte( uint16 index = 0 ) const;

      /**
       * Get buffer as a byte. Update the index.
       */
      byte incGetByte( uint32& index ) const;

      /**
       * Get buffer as an uint16.
       */
      uint16 getUint16( uint16 index = 0 ) const;

      /**
       * Get buffer as an uint16. Update the index.
       */
      uint16 incGetUint16( uint32& index ) const;

      /**
       * Get buffer as an int16.
       */
      int16 getInt16( uint16 index = 0 ) const;

      /**
       * Get buffer as an int16. Update the index.
       */
      int16 incGetInt16( uint32& index ) const;

      /**
       * Get an uint32 from a byte vector.
       */
      static uint32 getUint32FromByteArray( const byte* b,
                                            uint16 index = 0 );

      /**
       * Get buffer as an uint32. 
       */
      uint32 getUint32( uint16 index = 0 ) const;

      /**
       * Get buffer as an uint32. Update the index.
       */
      uint32 incGetUint32( uint32& index ) const;

      /**
       * Get buffer as an int32.
       */
      int32 getInt32( uint16 index = 0 ) const;

      /**
       * Get buffer as an int32. Update the index.
       */
      int32 incGetInt32( uint32& index ) const;

      /**
       * Get buffer as a string.
       */
      MC2String getString( bool usesLatin1 = false ) const;

      /**
       * Get buffer as a string. Update the index.
       */
      MC2String incGetString( bool usesLatin1, uint32& index ) const;

      /**
       * Get buffer as byte array. Is getLength bytes long.
       */
      const byte* getByteArray() const;


      /**
       * Get uint16 at position index.
       */
      uint16 getUint16Array( uint16 index ) const;


      /**
       * Get uint32 at position index.
       */
      uint32 getUint32Array( uint16 index ) const;


      /**
       * Get int32 at position index.
       */
      int32 getInt32Array( uint16 index ) const;


      /**
       * Append a byte.
       */
      void addByte( byte data );


      /**
       * Append an uint16.
       */
      void addUint16( uint16 data );


      /**
       * Append an uint32.
       */
      void addUint32( uint32 data );


      /**
       * Append an int32.
       */
      void addInt32( int32 data );


      /**
       * Append a byte array.
       */
      void addByteArray( const byte* buff, uint16 len );

      /**
       * Append a byte array.
       */
      void addByteArray( const char* buff, uint16 len );

      /**
       * Append a string.
       */
      void addString( const char* str, bool usesLatin1 = false );


      /**
       * Append a string.
       */
      void addString( const MC2String& str, bool usesLatin1 = false );


      /**
       * Write to a byte buffer.
       */
      void writeTo( vector< byte >& buff ) const;


      /**
       * Comparison operator <.
       */
      bool operator < ( const NParam& b ) const;


      /**
       * Comparison operator >.
       */
      bool operator > ( const NParam& b ) const;


      /**
       * Comparison operator !=.
       */
      bool operator != ( const NParam& b ) const;


      /**
       * Comparison operator ==.
       */
      bool operator == ( const NParam& b ) const;


      /**
       * Copy cont for debugprinting
       */
      NParam( const NParam& other );


      /**
       * Dump param to out.
       */
      void dump( ostream& out, bool dumpValue = false, 
                 bool singleLine = false, uint32 maxLen = 25 ) const;


      /**
       * Get the type as string.
       */
      static const char* getTypeAsString( NParam_t type );


      /**
       * Set the content.
       */
      void setBuff( const vector< byte > buff );


      /**
       * Get the buffer as vector< byte >.
       */
      const vector< byte >& getVector() const;


      /**
       * Clear the param from any content.
       */
      void clear();

      /**
       * Static method to add an uint32 to a byte array.
       */
      static void addUint32ToByteVector( vector<byte>& v, uint32 d );

      /**
       * Read param from dump string.
       */
      int readParam( const char* str );

   private:
      /**
       * Do not use this use the public one with bool usesLatin1.
       * Constructor from string -
       * Not implemented by choice, compiler happily makes bool of char*.
       */
      NParam( uint16 paramID, const char* data );


      /// The paramID.
      uint16 m_paramID;


      /// The buffer.
      vector<byte> m_buff;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline NParam::NParam( uint16 paramID, const byte* buff, uint16 len ) 
      : m_paramID( paramID ), m_buff( buff, buff + len )
{
}


inline NParam::NParam( const NParam& other ) {
   m_paramID = other.m_paramID;
   m_buff = other.m_buff;
}


inline void
NParam::addByte( byte data ) {
   m_buff.push_back( data );
}


inline void 
NParam::addUint16( uint16 data ) {
   m_buff.push_back( ( data ) >> 8 );
   m_buff.push_back( ( data ) & 0xff );
}

inline void
NParam::addUint32ToByteVector( vector<byte>& v, uint32 d ) {
   v.push_back( (( d ) >> 24 ) & 0xff );
   v.push_back( (( d ) >> 16 ) & 0xff );
   v.push_back( (( d ) >> 8  ) & 0xff );
   v.push_back( (( d ) )       & 0xff );
}

inline void 
NParam::addUint32( uint32 data ) {
   addUint32ToByteVector( m_buff, data );
}

inline void 
NParam::addInt32( int32 data ) {
   addUint32( data );
}


inline void 
NParam::addByteArray( const byte* buff, uint16 len ) {
   m_buff.insert( m_buff.end(), buff, buff + len );
}

inline void 
NParam::addByteArray( const char* buff, uint16 len ) {
   m_buff.insert( m_buff.end(), reinterpret_cast<const byte*>( buff ), 
                  reinterpret_cast<const byte*>( buff ) + len );
}

inline void
NParam::addString( const char* str, bool usesLatin1 ) {
   MC2String tmp;
   if( usesLatin1 ) {
      tmp = UTF8Util::mc2ToIso(MC2String(str));
   } else {
      tmp = UTF8Util::mc2ToUtf8(MC2String(str));
   }
   m_buff.insert( m_buff.end(), tmp.c_str(), tmp.c_str() + tmp.size() + 1 );
}


inline void
NParam::addString( const MC2String& str, bool usesLatin1 ) {
   MC2String tmp;
   if( usesLatin1 ) {
      tmp = UTF8Util::mc2ToIso(str);
   } else {
      tmp = UTF8Util::mc2ToUtf8(str);
   }
   m_buff.insert( m_buff.end(), tmp.c_str(), tmp.c_str() + tmp.size() + 1);
}


inline NParam::NParam( uint16 paramID, bool data ) 
      : m_paramID( paramID )
{
   if ( !data ) {
      m_buff.push_back( data );
   }
}


inline NParam::NParam( uint16 paramID, byte data ) 
      : m_paramID( paramID )
{
   m_buff.push_back( data );
}


inline NParam::NParam( uint16 paramID, uint16 data ) 
      : m_paramID( paramID )
{
   addUint16( data );
}


inline NParam::NParam( uint16 paramID, uint32 data ) 
      : m_paramID( paramID )
{
   addUint32( data );
}


inline NParam::NParam( uint16 paramID, int32 data ) 
      : m_paramID( paramID )
{
   addInt32( data );
}


inline NParam::NParam( uint16 paramID, const char* data, bool usesLatin1 )
      : m_paramID( paramID )
{
   addString( data, usesLatin1 );
}



inline NParam::NParam( uint16 paramID, const MC2String& data, 
                       bool usesLatin1 )
      : m_paramID( paramID )
{
   addString( data, usesLatin1 );
}


inline NParam::NParam( uint16 paramID, const uint16* data, uint16 len )
      : m_paramID( paramID )
{
   m_buff.reserve( len*2 );
   for ( uint16 i = 0 ; i < len ; ++i ) {
      addUint16( data[ i ] );
   }
}


inline NParam::NParam( uint16 paramID, const uint32* data, uint16 len )
      : m_paramID( paramID )
{
   m_buff.reserve( len*4 );
   for ( uint16 i = 0 ; i < len ; ++i ) {
      addUint32( data[ i ] );
   }
}


inline NParam::NParam( uint16 paramID, const int32* data, uint16 len )
      : m_paramID( paramID )
{
   m_buff.reserve( len*4 );
   for ( uint16 i = 0 ; i < len ; ++i ) {
      addInt32( data[ i ] );
   }
}


inline NParam::NParam( uint16 paramID, MC2Coordinate data )
      : m_paramID( paramID )
{
   Nav2Coordinate nav2Coord(data);
   addInt32( nav2Coord.nav2lat );
   addInt32( nav2Coord.nav2lon );
}


inline NParam::NParam( uint16 paramID )
      : m_paramID( paramID )
{
}


inline uint16
NParam::getParamID() const {
   return m_paramID;
}


inline void
NParam::setParamID( uint16 paramID ) {
   m_paramID = paramID;
}


inline uint16 
NParam::getLength() const {
   return m_buff.size();
}


inline const byte* 
NParam::getBuff() const {
   return &m_buff.front();
}


inline bool 
NParam::getBool() const {
   if ( getLength() == 0 ) {
      return true;
   } else {
      return m_buff[ 0 ] != 0;
   }
}

inline byte 
NParam::getByte( uint16 index ) const {
   return m_buff[ index ];
}

inline byte 
NParam::incGetByte( uint32& index ) const {
   return m_buff[ index++ ];
}

inline uint16 
NParam::getUint16( uint16 index ) const {
   return ntohs(*((uint16 *)(&(m_buff.front())+index)));
}

inline uint16 
NParam::incGetUint16( uint32& index ) const {
   uint16 r = ntohs(*((uint16 *)(&(m_buff.front())+index)));
   index += 2;
   return r;
}

inline int16 
NParam::getInt16( uint16 index ) const {
   return ntohs(*((int16 *)(&(m_buff.front())+index)));
}

inline int16 
NParam::incGetInt16( uint32& index ) const {
   int16 r = ntohs(*((int16 *)(&(m_buff.front())+index)));
   index += 2;
   return r;
}

inline uint32
NParam::getUint32FromByteArray( const byte* b, uint16 index ) {
   return ntohl(*((uint32 *)(b+index)));
}

inline uint32 
NParam::getUint32( uint16 index ) const {
   return getUint32FromByteArray( &m_buff.front(), index );
}

inline uint32 
NParam::incGetUint32( uint32& index ) const {
   uint32 r = getUint32FromByteArray( &m_buff.front(), index );
   index += 4;
   return r;
}

inline int32 
NParam::getInt32( uint16 index ) const {
   return ntohl(*((int32 *)(&(m_buff.front())+index)));
}

inline int32 
NParam::incGetInt32( uint32& index ) const {
   int32 r = ntohl(*((int32 *)(&(m_buff.front())+index)));
   index += 4;
   return r;
}

inline MC2String 
NParam::getString( bool usesLatin1 ) const {
   MC2String str = reinterpret_cast<const char*> (&m_buff.front());
   if( usesLatin1 ) {
      return UTF8Util::isoToMc2(str);
   } else {
      return UTF8Util::utf8ToMc2(str);
   }
}


inline MC2String 
NParam::incGetString( bool usesLatin1, uint32& index ) const {
   MC2String str = reinterpret_cast<const char*> (&m_buff.front() + index);
   index += str.length() + 1;
   if( usesLatin1 ) {
      return UTF8Util::isoToMc2(str);
   } else {
      return UTF8Util::utf8ToMc2(str);
   }
}


inline const byte* 
NParam::getByteArray() const {
   return &m_buff.front();
}


inline uint16
NParam::getUint16Array( uint16 index ) const {
   return ntohs(*(((uint16 *)&(m_buff.front()))+index));
}


inline uint32
NParam::getUint32Array( uint16 index ) const {
   return ntohl(*(((uint32 *)&(m_buff.front()))+index));
}


inline int32
NParam::getInt32Array( uint16 index ) const {
   return ntohl(*(((int32 *)&(m_buff.front()))+index));
}


inline void
NParam::writeTo( vector< byte >& buff ) const {
   buff.push_back( ( m_paramID ) >> 8 );
   buff.push_back( ( m_paramID ) & 0xff );
   buff.push_back( ( m_buff.size() ) >> 8 );
   buff.push_back( ( m_buff.size() ) & 0xff );
   buff.insert( buff.end(), m_buff.begin(), m_buff.end() );
}


inline bool 
NParam::operator < ( const NParam& b ) const {
   return m_paramID < b.m_paramID;
}


inline bool 
NParam::operator > ( const NParam& b ) const {
   return m_paramID > b.m_paramID;
}


inline bool 
NParam::operator != ( const NParam& b ) const {
   return m_paramID != b.m_paramID;
}


inline bool 
NParam::operator == ( const NParam& b ) const {
   return m_paramID == b.m_paramID;
}


inline void
NParam::setBuff( const vector< byte > buff ) {
   m_buff = buff;
}


inline const vector< byte >&
NParam::getVector() const {
   return m_buff;
}


inline void
NParam::clear() {
   m_buff.clear();
}


#endif // NPARAM_H

