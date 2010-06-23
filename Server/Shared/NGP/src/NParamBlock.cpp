/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NParamBlock.h"
#include "TimeUtility.h"
#include "Utility.h"
#include "GzipUtil.h"
#include "GunzipUtil.h"
#include "StringUtility.h"

const uint32 NParamBlock::maxMsgSize = 2131072;


NParamBlock::NParamBlock()
      : m_find( 0 ), m_valid( true ), m_flags( 0 )
{
}


NParamBlock::NParamBlock( const byte* buff, uint32 len, byte protVer,
                          uint32* uncompressedSize ) 
      : m_find( 0 ), m_valid( true )
{
   // Read and add NParams
   uint32 pos = 0;
   m_flags = 0;
   vector< byte > gzBuff;
   if ( uncompressedSize ) {
      *uncompressedSize = len;
   }

   if ( protVer >= 0xc && pos + 4 <= len ) {
      // Read flags
      m_flags = NParam::getUint32FromByteArray( buff, pos ); pos += 4;
      uint32 tflag = m_flags;
      while ( (tflag & 0x80000000) && pos + 4 <= len ) {
         tflag = NParam::getUint32FromByteArray( buff, pos ); pos += 4;
      }
      if ( m_flags & 0x1 ) {
         // CRC and length last in buffer
         uint32 startTime = TimeUtility::getCurrentMicroTime();
         if ( GunzipUtil::check_header( buff + pos, len - pos ) ) {
            int realLength = GunzipUtil::origLength( buff + pos, 
                                                     len - pos );
            int gzRes = -1;
            if ( realLength <= int32(maxMsgSize) ) {
               gzBuff.reserve( realLength );
               gzRes = GunzipUtil::gunzip( &gzBuff.front(), realLength,
                                           buff + pos, len - pos );
            }
            uint32 stopTime = TimeUtility::getCurrentMicroTime();
            if ( gzRes >= 0 ) {
               mc2dbg4 << "Gunziped " << (len - pos) << " bytes into " 
                      << realLength << " bytes in " 
                      << (stopTime - startTime) << "us"<< endl;
               if ( uncompressedSize ) {
                  *uncompressedSize = pos + realLength;
               }
               buff = &gzBuff.front();
               len = realLength;
               pos = 0;
            } else {
               mc2log << warn << "NParamBlock::NParamBlock " 
                      << (len - pos) << " Not ok gzip " << gzRes
                      << " uncompressed size " << realLength 
                      << ". Buffer: " << endl;
               Utility::hexDump( mc2log, const_cast<byte*>( buff ), len );
               pos = len;
               m_valid = false;
            }
         } else {
            mc2log << warn << "NParamBlock::NParamBlock " << (len - pos) 
                   << " Not ok gzip header. Buffer: " << endl;
            Utility::hexDump( mc2log, const_cast<byte*>( buff ), len );
            pos = len;
            m_valid = false;
         }
      }
   }

   while ( pos + 4 <= len ) {
      uint16 paramID = ntohs( *((uint16*)(buff + pos)) );
      pos += 2;
      uint16 length = ntohs( *((uint16*)(buff + pos)) );
      pos += 2;

      if ( pos + length <= len ) {
         NParam& p = addParam( NParam( paramID ) );
         p.addByteArray( buff + pos, length );
         pos += length;
      } else {
         mc2log << warn << "NParamBlock::NParamBlock param " << paramID
                << " len " << length << " too large " << (pos + length)
                << " >= " << len << endl;
         Utility::hexDump( mc2log, const_cast<byte*>( buff ), len );
         pos = len;
         m_valid = false;
      }
   }
   if ( pos != len ) {
      mc2log << warn << "NParamBlock::NParamBlock " << (len - pos) 
             << " trailing bytes. Buffer: " << endl;
      Utility::hexDump( mc2log, const_cast<byte*>( buff ), len );
      m_valid = false;
   }
}


const NParam* 
NParamBlock::getParam( uint16 paramID ) const {
   m_find.setParamID( paramID );
   NParams::const_iterator it = m_params.find( m_find );
   if ( it != m_params.end() ) {
      return &*it;
   } else {
      return NULL;
   }
}


void 
NParamBlock::getAllParams( uint16 paramID, 
                           vector< const NParam* >& params ) const
{
   m_find.setParamID( paramID );
   pair<NParams::const_iterator, NParams::const_iterator> res =
      m_params.equal_range( m_find );
   for ( NParams::const_iterator it = res.first ; it != res.second ; ++it )
   {
      params.push_back( &*it );
   }
}


void
NParamBlock::writeParams( vector< byte >& buff, byte protVer,
                          bool mayUseGzip, uint32* uncompressedSize ) const 
{
   uint32 flags = 0x0;
   uint32 startSize = buff.size();
   if ( protVer >= 0xc ) {
      // Supports gzip
      if ( GunzipUtil::implemented() ) {
         flags |= 0x2;  
      }
      NParam::addUint32ToByteVector( buff, flags );
   }
   for ( NParams::const_iterator it = m_params.begin() ; 
         it != m_params.end() ; ++it )
   {
      (*it).writeTo( buff );
   }
   if ( uncompressedSize ) {
      *uncompressedSize = buff.size();
   }
   if ( flags & 0x2 && mayUseGzip ) {
      // Try gzip
      uint32 startTime = TimeUtility::getCurrentMicroTime();
      vector<byte> gzBuff( buff.begin(), buff.begin() + startSize );
      NParam::addUint32ToByteVector( gzBuff, flags | 0x1 ); /* gziped */
      uint32 flagsSize = 4;
      gzBuff.resize( buff.size() );
      mc2dbg4 << "gzBuff.size() " << gzBuff.size() << endl;
      int gres = GzipUtil::gzip( &gzBuff.front() + startSize + flagsSize, 
                                 gzBuff.size()   - startSize - flagsSize,
                                 &buff.front()   + startSize + flagsSize, 
                                 buff.size()     - startSize - flagsSize );
      uint32 stopTime = TimeUtility::getCurrentMicroTime();
      if ( gres > 0 ) {
         gzBuff.erase( gzBuff.begin() + gres + startSize + flagsSize, 
                       gzBuff.end() );
         mc2dbg4 << "Gziped " << (buff.size() - 4) << " bytes into "
                << gzBuff.size() << " bytes in " 
                << (stopTime - startTime) << "us" << endl;
//          Utility::hexDump( mc2log, const_cast<byte*>( &gzBuff.front() ),
//                            gzBuff.size() );
         buff.swap( gzBuff );
      } else {
         mc2dbg4 << "Gziped failed res " << gres << " size " 
                << gzBuff.size() << " in " 
                << (stopTime - startTime) << "us" << endl;
      }
   }
}


NParam& 
NParamBlock::addParam( const NParam& param ) {
   return const_cast< NParam& >( *m_params.insert( param ) );
}


NParam& 
NParamBlock::updateParam( const NParam& param ) {
   m_find.setParamID( param.getParamID() );
   NParams::iterator it = m_params.find( m_find );
   if ( it != m_params.end() ) {
      const_cast< NParam& >( *it ).setBuff( param.getVector() );
      return const_cast< NParam& >( *it );
   } else {
      return addParam( param );
   }
}


void
NParamBlock::removeParam( uint16 paramID ) {
   m_params.erase( paramID );
}


void 
NParamBlock::dump( ostream& out, bool dumpValues, bool singleLine, 
                   uint32 maxLen, uint32 maxSame ) const 
{
   out << "NParamBlock " << m_params.size() << " params ";
   if ( !singleLine ) {
      out << endl;
   }
   uint32 lastId = 0;
   uint32 nbrSameId = 0;
   for ( NParams::const_iterator it = m_params.begin() ; 
         it != m_params.end() ; /*++it*/ )
   {
      if ( (*it).getParamID() == lastId ) {
         ++nbrSameId;
      } else {
         nbrSameId = 0;
      }
      lastId = (*it).getParamID();
      (*it).dump( out, dumpValues, singleLine, 
                  nbrSameId > maxSame ? 0 : maxLen );
      ++it;
      if ( singleLine && it != m_params.end() ) {
         out << " ";
      }
   }
   if ( singleLine ) {
      out << endl;
   }
}

bool
NParamBlock::readParams( const char* str, NParamBlock& params, byte& protoVer, 
                         uint16& type, byte& reqID, byte& reqVer ) {
   // NavRequest protoVer 0xc type 0x46 reqID 0x0 reqVer 0x1 NParamBlock 5 
   // params 34:wf-s-60-v5-demo. 5:none. 6:0x0001 28:NoCRC!.

   bool ok = true;
   // Header NavRequest
   MC2String reqStart( "NavRequest" );
   char* startChar = StringUtility::strstr( str, reqStart.c_str() );
   if ( startChar != NULL ) {
      char tmp[ strlen( str ) + 1 ];
      uint32 pos = startChar - str + reqStart.size() + 1;
      int res = 1;
      int sprotoVer, stype, sreqID, sreqVer;
      if ( res > 0 ) {
         res = sscanf( str + pos, 
                       "protoVer %X type %X reqID %X reqVer %X "
                       "NParamBlock %s", 
                       &sprotoVer, &stype, &sreqID, &sreqVer, tmp );
      }
      if ( res == 5 ) {
         protoVer = sprotoVer;
         type = stype;
         reqID = sreqID;
         reqVer = sreqVer;

         // Params
         MC2String paramStart( "NParamBlock" );
         startChar = StringUtility::strstr( str, paramStart.c_str() );
         if ( startChar != NULL ) {
            pos = startChar - str + paramStart.size() + 1;
            int nbrParams = 0;
            res = sscanf( str + pos, 
                          "%d params %s", 
                          &nbrParams, tmp );
            if ( res == 2 ) {
               MC2String data( str );
               // For each word... But space in strings!
               // Find ':' then backup to space...
               while ( pos + 1 < data.size() ) {
                  MC2String::size_type colonPos = data.find( ':', pos );
                  MC2String::size_type nextColonPos = data.find( 
                      ':', colonPos + 1 );
                  MC2String::size_type paramIDPos = MC2String::npos;
                  MC2String::size_type nextParamIDPos = MC2String::npos;
                  if ( colonPos != MC2String::npos ) {
                     paramIDPos = data.rfind( ' ', colonPos );
                     if ( paramIDPos != MC2String::npos ) {
                        ++paramIDPos;
                     }
                  }
                  if ( nextColonPos == MC2String::npos ) {
                     nextColonPos = data.size() - 1;
                     nextParamIDPos = data.size();
                  } else {
                     nextParamIDPos = data.rfind( ' ', nextColonPos );
                     if ( nextParamIDPos == MC2String::npos ) {
                        nextParamIDPos = data.size();
                     }
                  }
                        
                  if ( paramIDPos != MC2String::npos ) {
                     NParam p( 0 );
                     int res = p.readParam( 
                        data.substr( paramIDPos,
                                     (nextParamIDPos - paramIDPos) ).c_str() );
                     if ( res > 0 ) {
                        params.addParam( p );
                        pos = paramIDPos + res;
                     } else {
                        mc2dbg/*4*/ << "Bad param " 
                                << data.substr( paramIDPos,
                                                (nextParamIDPos - paramIDPos) )
                                << endl;
                        pos = MAX_UINT32;
                     }
                  } else {
                     mc2dbg/*4*/ << "Bad paramIDPos" << endl;
                     pos = MAX_UINT32;
                  }
                  
                  if ( pos != MAX_UINT32 ) {
                     while( pos < data.size() && 
                            isspace( data[ pos ] ) ) {
                        ++pos;
                     }
                  }
               } // End while pos in string
               
               if ( pos == MAX_UINT32 ) {
                  ok = false;
                  mc2dbg/*4*/ << "loadParamFile Bad NParamBlock." << endl;
               }
            } else {
               ok = false;
               mc2dbg/*4*/ << "loadParamFile Bad NParamBlock start." << endl;
            }
         } else {
            ok = false;
            mc2dbg/*4*/ << "loadParamFile No NParamBlock in file." << endl;
         }
      } else {
         ok = false;
         mc2dbg/*4*/ << "loadParamFile Bad start of NavRequest." << endl;
      }
   } else {
      ok = false;
      mc2dbg/*4*/ << "loadParamFile No NavRequest in file." << endl;
   }

   return ok;
}


bool
NParamBlock::mayUseGzip() const {
   return m_flags & 0x2;
}
