/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "XSData.h"
#include "HttpBody.h"
#include "GzipUtil.h"
#include "GunzipUtil.h"
#include "StackOrHeap.h"


namespace XSData {
// Data
static const byte xorKey[256] = {
   0x4c, 0x87, 0x3f, 0x4f, 0xea, 0xa8, 0xa4, 0xb7, 0xb4, 0x13, 0xba, 0x4b,
   0xea, 0xa8, 0xa4, 0xb7, 0xb4, 0x13, 0xba, 0x4b, 0x54, 0x58, 0xc7, 0x01,
   0xb4, 0x13, 0xba, 0x4b, 0x54, 0x58, 0xc7, 0x01, 0xb5, 0xc9, 0x89, 0x03,
   0x54, 0x58, 0xc7, 0x01, 0xb5, 0xc9, 0x89, 0x03, 0x77, 0x78, 0xe4, 0x46,
   0xb5, 0xc9, 0x89, 0x03, 0x77, 0x78, 0xe4, 0x46, 0xb4, 0xe1, 0x91, 0x25,
   0x77, 0x78, 0xe4, 0x46, 0xb4, 0xe1, 0x91, 0x25, 0x96, 0x63, 0xf8, 0xe1,
   0xb4, 0xe1, 0x91, 0x25, 0x96, 0x63, 0xf8, 0xe1, 0xea, 0x37, 0x2f, 0xd4,
   0x96, 0x63, 0xf8, 0xe1, 0xea, 0x37, 0x2f, 0xd4, 0xde, 0xd3, 0x8a, 0x92,
   0xea, 0x37, 0x2f, 0xd4, 0xde, 0xd3, 0x8a, 0x92, 0xe6, 0x43, 0xdc, 0x39,
   0xde, 0xd3, 0x8a, 0x92, 0xe6, 0x43, 0xdc, 0x39, 0x9a, 0xa2, 0x3a, 0x4e,
   0xe6, 0x43, 0xdc, 0x39, 0x9a, 0xa2, 0x3a, 0x4e, 0x6b, 0xc3, 0x51, 0xe2,
   0x9a, 0xa2, 0x3a, 0x4e, 0x6b, 0xc3, 0x51, 0xe2, 0x3a, 0x34, 0x27, 0xee,
   0x6b, 0xc3, 0x51, 0xe2, 0x3a, 0x34, 0x27, 0xee, 0x15, 0xb8, 0x12, 0xaa,
   0x3a, 0x34, 0x27, 0xee, 0x15, 0xb8, 0x12, 0xaa, 0x1b, 0x0a, 0x8a, 0x04,
   0x15, 0xb8, 0x12, 0xaa, 0x1b, 0x0a, 0x8a, 0x04, 0x40, 0xb9, 0xd7, 0x1e,
   0x1b, 0x0a, 0x8a, 0x04, 0x40, 0xb9, 0xd7, 0x1e, 0x8c, 0x60, 0xaf, 0x71,
   0x40, 0xb9, 0xd7, 0x1e, 0x8c, 0x60, 0xaf, 0x71, 0xa2, 0x8a, 0xaa, 0x3c,
   0x8c, 0x60, 0xaf, 0x71, 0xa2, 0x8a, 0xaa, 0x3c, 0x2c, 0xe3, 0x8a, 0x96,
   0xa2, 0x8a, 0xaa, 0x3c, 0x2c, 0xe3, 0x8a, 0x96, 0xa5, 0xdb, 0x77, 0xdf,
   0x2c, 0xe3, 0x8a, 0x96, 0xa5, 0xdb, 0x77, 0xdf, 0x0e, 0x9e, 0xcc, 0x22,
   0xa5, 0xdb, 0x77, 0xdf, 0x0e, 0x9e, 0xcc, 0x22, 0x55, 0xde, 0xcc, 0x6f,
   0x0e, 0x9e, 0xcc, 0x22, 
};
static const uint32 xorKeyLen = 256;

void decodeXSBuffer( byte* buff, uint32 len ) {
   uint32 magicPos = 0;
   for ( uint32 i = 0 ; i < len ; i++ ) {
      buff[ i ] = buff[ i ] ^ xorKey[ magicPos ];
      magicPos++;
      if ( magicPos >= xorKeyLen ) {
         magicPos = 0;
      }
   }
}

void decodeXSBuffer( HttpBody& body ) {
   // unxor
   decodeXSBuffer( (byte*)body.getBody(), body.getBodyLength() );

   // ungzip
   if ( body.getBodyLength() >= 2 && 
        GunzipUtil::isGzip( reinterpret_cast<const byte*> ( 
                               body.getBody() ) ) ) {
      const uint32 inBodyLength = body.getBodyLength();
      uint32 ungzipLen = GunzipUtil::origLength( 
         reinterpret_cast<const byte*> ( body.getBody() ), inBodyLength ); 
      MC2String buff;
      if ( ungzipLen > 50000000 ) {
         mc2log << warn << "[XSData::decodeXSBuffer] Gzipped original "
                << "length is too large " << ungzipLen << endl;
         ungzipLen = 50000000;
      }
      buff.resize( ungzipLen );
      if ( GunzipUtil::gunzip( (byte*)buff.data(), buff.size(), 
                               (const byte*) body.getBody(),
                               inBodyLength ) >= 0 ) {
         body.setBody( reinterpret_cast<const byte*> ( buff.data() ), 
                       buff.size() );
      } else {
         mc2log << warn << "[XSData::decodeXSBuffer] GunZip failed "
                << "using raw body." << endl;
      }
   }
}

void encodeXSBuffer( HttpBody& body ) {
   // gzip
   StackOrHeap< 256, byte > buff( body.getBodyLength() );
   int gres = GzipUtil::gzip( 
      buff, body.getBodyLength(), 
      reinterpret_cast<const byte*>(body.getBody()),
      body.getBodyLength() );
   if ( gres >= 0 ) {
      // Ok use the gziped one
      // Only the gzipped bytes
      body.setBody( buff, gres );
   } else {
      // No gzip possible, use the original body
   }

   // xor
   decodeXSBuffer( (byte*)body.getBody(), body.getBodyLength() );

   // Set binary in body
   body.setBinary( true );
}


} // namespace XSData

