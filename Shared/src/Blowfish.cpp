/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "Blowfish.h"

#include "ScopedArray.h"
#include "DataBuffer.h"
#include "TimeUtility.h"
#include "StringUtility.h"

#include <openssl/evp.h>
#include <openssl/blowfish.h>


namespace Blowfish {

auto_ptr<DataBuffer> encrypt( const char* key, const DataBuffer& data ) {
   // generate iv vector from current time

   DataBuffer iv( 9 );
   iv.fillWithZeros();
   iv.writeNextLong( TimeUtility::getCurrentTime() );

   auto_ptr<DataBuffer> buf( encrypt( key, (const char*)iv.getBufferAddress(),
                                      data ) );
   if ( buf.get() == NULL ) {
      return buf;
   }

   // prepend iv vector to data

   // create a new buffer with large size so iv vector can be included
   auto_ptr<DataBuffer> 
      finalBuf( new DataBuffer( buf->getBufferSize() + 
                                iv.getBufferSize() - 1) );

   // copy iv vector
   memcpy( finalBuf->getBufferAddress(), 
           iv.getBufferAddress(), iv.getBufferSize() - 1);

   // copy encrypted data
   memcpy( finalBuf->getBufferAddress() + iv.getBufferSize() - 1, 
           buf->getBufferAddress(), buf->getBufferSize() );

   return finalBuf;
}

auto_ptr<DataBuffer> decrypt( const char* key, const DataBuffer& data ) {
   // must at lest be 8 bytes long
   if ( data.getBufferSize() <= 8 ) {
      return auto_ptr<DataBuffer>();
   }

   char iv[ 9 ]; // one extra to pad with 0
   iv[ 8 ] = 0;
   for ( uint32 i = 0; i < 8; ++i ) {
      iv[ i ] = data.getBufferAddress()[ i ];
   }

   // decrypt from current offset
   return decrypt( key, iv, DataBuffer( data.getBufferAddress() + 8, 
                                        data.getBufferSize() - 8 ) );
}

auto_ptr<DataBuffer> encrypt( const char* key, const char* iv,
                              const DataBuffer& data ) {

   // initialize context struct for blowfish cbc
   EVP_CIPHER_CTX ctx;

   EVP_CIPHER_CTX_init( &ctx );
   EVP_EncryptInit( &ctx, EVP_bf_cbc(), 
                    (unsigned char*)key, 
                    (unsigned char*)iv );

   // the size of output data ranges from 
   // 0 to ( str.size() + cipher_block_size - 1 )
   // the encrypt final must have at least one cipher block size of space
   // so we allocate one extra block for that, thus 2 * cipher_block_size
   ScopedArray<unsigned char> 
      outbuf( new unsigned char[ data.getBufferSize() + 
                                 2*EVP_CIPHER_CTX_block_size( &ctx )]);
   int outlen = 0;

   // encrypt string
   if ( ! EVP_EncryptUpdate( &ctx,
                             outbuf.get(), &outlen, 
                             (unsigned char*)data.getBufferAddress(), 
                             data.getBufferSize() ) ) {

      EVP_CIPHER_CTX_cleanup( &ctx );
      // failed to encrypt string
      return auto_ptr<DataBuffer>();
   }

   // finalize encryption
   //
   // Buffer passed to EVP_EncryptFinal() must be after data just
   // encrypted to avoid overwriting it.
   int tmplen = 0;
   if ( ! EVP_EncryptFinal( &ctx, 
                            outbuf.get() + outlen, 
                            &tmplen ) ) {
      EVP_CIPHER_CTX_cleanup( &ctx );
      // failed to finilize encryption
      return auto_ptr<DataBuffer>();
   }
   EVP_CIPHER_CTX_cleanup( &ctx );

   // final buffer size
   outlen += tmplen;

   auto_ptr<DataBuffer> retBuf(new DataBuffer( outbuf.release(), 
                                               outlen ) );
   retBuf->setSelfAlloc( true ); // we now own this buffer
   return retBuf;
}

auto_ptr<DataBuffer> decrypt( const char* key, const char* iv, 
                              const DataBuffer& buff ) {
   int outlen = 0;

   unsigned char *input = (unsigned char*)buff.getBufferAddress();
   int lenEncrypted = buff.getBufferSize();

   ScopedArray<unsigned char> 
      decodebuffer( new unsigned char[ buff.getBufferSize() ] );

   // initialize context struct to blowfish cbc
   EVP_CIPHER_CTX ctx;
   EVP_CIPHER_CTX_init(&ctx);
   EVP_DecryptInit( &ctx, EVP_bf_cbc(), 
                    (unsigned char*)key, 
                    (unsigned char*)iv );

   if( ! EVP_DecryptUpdate( &ctx, decodebuffer.get(), &outlen,
                            input, lenEncrypted ) ) {
      
      EVP_CIPHER_CTX_cleanup( &ctx );
      return std::auto_ptr<DataBuffer>();
   }

   // finalize decryption
   int tmplen = 0;
   if ( !EVP_DecryptFinal( &ctx, 
                           decodebuffer.get() + outlen, 
                           &tmplen ) ) {
       EVP_CIPHER_CTX_cleanup( &ctx );
       // failed to decrypt final
       return std::auto_ptr<DataBuffer>();
   }
   EVP_CIPHER_CTX_cleanup( &ctx );

   // final size
   outlen += tmplen;

   auto_ptr<DataBuffer> retBuf( new DataBuffer( decodebuffer.release(),
                                                outlen ) );
   retBuf->setSelfAlloc( true ); // wo now own this buffer
   return retBuf;
}

namespace PJ {

MC2String encrypt( const MC2String& key, const vector<char>& ivIn,
                   const MC2String& str ) {

   // check argument values
   if ( ivIn.size() < 8 || key.empty() ) {
      return "";
   }

   // blowfish works with 64 bits ( 8 bytes ) block
   const uint32 blockSize = 8;

   // copy iv since it will be modified by blowfish
   // and only copy block size number of bytes
   vector<unsigned char> iv( ivIn.begin(), ivIn.begin() + blockSize );

   // setup key
   BF_KEY bfkey;
   BF_set_key( &bfkey, key.size(), (unsigned char*)key.c_str() );



   // alignt outdata for blocksize
   const uint32 outDataMaxSize =  
      str.size() / blockSize * blockSize 
      + blockSize;

   unsigned char outdata[ outDataMaxSize ];
   memset( outdata, 0, outDataMaxSize );

   /* 
    * encrypt one block at the time
    */
   uint32 outSize = 0;
   for ( uint32 i = 0; i < str.size(); i += blockSize ) {
      MC2String blockStr = str.substr( i, blockSize );
      vector<unsigned char> block( blockStr.begin(), blockStr.end() );

      // fill block to blockSize by inserting 0 in the beginning
      if ( block.size() < blockSize ) {
         vector<unsigned char> newBlock( blockSize );
         fill( newBlock.begin(), newBlock.end(), 0 );
         copy( block.begin(), block.end(),
               newBlock.end() - block.size() );
         block = newBlock;

      }
      BF_cbc_encrypt( &block[0], 
                      outdata + i, block.size(), 
                      &bfkey, &iv[0], 
                      BF_ENCRYPT );
      outSize += blockSize;
   }
   ScopedArray<char> encodedData( new char[ outSize + outSize/ 3 + 4 ] );
   StringUtility::base64Encode( (byte*)outdata, outSize, encodedData.get() );
   return encodedData.get();
}
}

}
