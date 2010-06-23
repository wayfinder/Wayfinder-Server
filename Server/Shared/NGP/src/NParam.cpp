/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NParam.h"
#include "Utility.h"

void
NParam::dump( ostream& out, bool dumpValue, bool singleLine, 
              uint32 maxLen ) const
 {
   if ( !singleLine ) {
      out << "   ParamID " << getParamID() << " len " 
          << getLength() << endl;
      if ( dumpValue ) {
         Utility::hexDump( out, const_cast<byte*>( getBuff() ), 
                           getLength(), "      " );
      }
   } else {
      out << getParamID();
      bool allPrintable = true;
      bool anyPrintable = false;
      for ( uint32 i = 0 ; i < MIN( getLength(), maxLen ) ; ++i ) {
         if ( !isprint( getVector()[ i ] ) || getVector()[ i ] == ':' ||
              getVector()[ i ] == ';' ) {
            if ( i + 1 == getLength() && getVector()[ i ] == '\0' &&
                 getLength() > 1 )
            {
               // Ok is stringlike
            } else {
               allPrintable = false;
            }
         } else {
            anyPrintable = true; 
         }
      }
      if ( !allPrintable ) {
         out << ":0x" << hex << setfill( '0' );
         for ( uint32 i = 0 ; i < MIN( getLength(), maxLen ) ; ++i ) {
            out << setw( 2 ) << uint32( getVector()[ i ] );
         }
         out << dec;
         out << ";";
      }
      if ( anyPrintable ) {
         if ( allPrintable ) {
            out << ":";
         }
         for ( uint32 i = 0 ; i < MIN( getLength(), maxLen ) ; ++i ) {
            if ( isprint( getVector()[ i ] ) && getVector()[ i ] != ':' &&
                 getVector()[ i ] != ';' ) {
               out << getVector()[ i ];
            } else {
               out << ".";
            }
         }
      } else if ( getLength() == 0 ) {
         // Empty
         out << ":";
      }
      if ( getLength() > maxLen ) {
         out << "(" << getLength() << ")"; 
      }
   }
}

int
NParam::readParam( const char* str ) {
   mc2dbg4 << "str " << str << endl;
   int res = 0;
   // 4:wf-s-60-v5-demo. 5:none. 6:0x0001 28:NoCRC!.
   const uint32 strLen = strlen( str ) + 1;
   char data[ strLen ];
   char tmp[ strLen ];
   tmp[ 0 ] = '\0';
   uint16 paramID = 0;
   uint32 extraLen = 0;
   char tmpChar = '\0';
   bool isHex = false;
   if ( sscanf( str, "%hu:0x%[0-9A-Fa-f];%[^\n]", &paramID, data + 2, tmp ) 
        == 3 ) {
      data[ 0 ] = '0';
      data[ 1 ] = 'x';
      extraLen += 1 + strlen( tmp );
      res = 1;
      isHex = true;
      mc2dbg4 << "Is HEX with some printable" << endl;
   } else if ( sscanf( str, "%hu:0x%[0-9A-Fa-f]%c", &paramID, data + 2, 
                       &tmpChar ) == 3 && tmpChar == ';' )
   {
      data[ 0 ] = '0';
      data[ 1 ] = 'x';
      extraLen += 1;
      res = 1;
      isHex = true;
      mc2dbg4 << "Is HEX" << endl;
   } else if ( sscanf( str, "%hu:%[^\n]", &paramID, data ) == 2 ) {
      mc2dbg4 << "Is bytes" << endl;
      res = 1;
   } else if ( sscanf( str, "%hu:%[^\n]", &paramID, data ) == 1 ) {
      mc2dbg4 << "Is empty param" << endl;
      data[ 0 ] = '\0';
      res = 1;
   } else {
      mc2dbg4 << "Is no type!" << endl;
      res = 0;
   }

   mc2dbg4 << "paramID " << paramID << " data " << data << endl;
   if ( res > 0 ) {
      m_paramID = paramID;
      const uint32 dataLen = strlen( data );
      uint32 paramIDLen = 1;
      // Improve this only early test
      while ( paramID >= 10 ) {
         ++paramIDLen;
         paramID /= 10;
      }
      res = dataLen + extraLen + 1 + paramIDLen;
      if ( isHex && dataLen > 1 && data[ 0 ] == '0' && data[ 1 ] == 'x' ) {
         // Hex
         m_buff.clear();
         int pos = 2; // 0x
         while ( data[ pos ] != '\0' && data[ pos ] != ';' ) {
            byte a = isdigit( data[ pos ] ) ? (data[ pos ]-48) :
               (isupper( data[ pos ] ) ? (data[ pos ]-55) : 
                (data[ pos ]-87));
            byte b = isdigit( data[ pos + 1 ] ) ? (data[ pos + 1 ]-48) :
               (isupper( data[ pos + 1 ] ) ? (data[ pos + 1 ]-55) : 
                (data[ pos + 1 ]-87));
            m_buff.push_back( (a<<4) + b );
            pos += 2;
         }
      } else if ( dataLen > 0 && data[ dataLen -1 ] == '.' ) {
         m_buff.insert( m_buff.begin(), data, data + dataLen );
         m_buff.back() = '\0';
      } else {
         m_buff.insert( m_buff.begin(), data, data + dataLen );
      }
   }

   return res;
}


const char*
NParam::getTypeAsString( NParam_t type ) {
   switch ( type ) {
      case Bool :
         return "Bool";
      case Byte :
         return "Byte";
      case Uint16 :
         return "Uint16";
      case Uint32 :
         return "Uint32";
      case Int32 :
         return "Int32";
      case String :
         return "String";
      case Byte_array :
         return "Byte_array";
      case Uint16_array :
         return "Uint16_array";
      case Uint32_array :
         return "Uint32_array";
      case Int32_array :
         return "Int32_array";
   }

   return "Unknown type";
}
