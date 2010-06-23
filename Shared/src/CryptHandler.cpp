/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "CryptHandler.h"

#include "ScopedArray.h"

#if defined(USE_SSL) || defined(USE_CRYPTO)

void
CryptHandler::initialize() {
   OpenSSL_add_all_algorithms();
}


byte*
CryptHandler::encryptBuffer( const byte* const inBuff, uint32 len,
                             const char* const key, int& outl )
{
   DEBUG4(cerr << "encryptBuffer( " <<  len << " ) " << endl;);

   EVP_CIPHER_CTX* cCtx;
   const EVP_CIPHER *c;

   const char* iv = "ahfy573E"; // Must be same in enc and dec
                               // not used with all cryptos (not with rc4)

   cCtx = ( EVP_CIPHER_CTX *)OPENSSL_malloc(sizeof(EVP_CIPHER_CTX));

   c = EVP_rc4(); // Default 128 bit

   EVP_CIPHER_CTX_init( cCtx );

   DEBUG8(cerr << " keyl " << EVP_CIPHER_key_length( c ) << endl
          << "iv length " << EVP_CIPHER_iv_length( c ) << endl
          << "max keyl " <<  EVP_MAX_KEY_LENGTH << endl;);

   
   if ( ! EVP_EncryptInit( cCtx, c, (unsigned char*)key, 
                           (unsigned char*)iv ) )
   {
      MC2WARNING( 
         "CryptHandler::encryptBuffer EVP_EncryptInit failed" );
      return NULL;  
   }

   // Enough space in out buffer for an additional block
   outl = len + EVP_MAX_BLOCK_LENGTH; 
   // EVP_ENCODE_LENGTH( len ); Is for base64 encoded data
   ScopedArray<byte> out( new byte[ outl ] );

   if ( ! EVP_EncryptUpdate( cCtx, out.get(), &outl, 
                             (unsigned char*)inBuff, len ) )
   {
      MC2WARNING( "CryptHandler::encryptBuffer EncryptUpdate failed" );
      return NULL;
   }

   int finall = 0;
   // The padding if any 
   if ( ! EVP_EncryptFinal( cCtx, out.get() + outl, &finall ) ) {
      MC2WARNING( "CryptHandler::encryptBuffer EncryptFinal failed" ) ;
      return NULL;
   }

   outl += finall;

   DEBUG8(cerr << "EncryptFinal done outl " << outl << endl;);

   EVP_CIPHER_CTX_cleanup( cCtx );

   return out.release();
}


byte*
CryptHandler::decryptBuffer( const byte* const inBuff, uint32 len,
                             const char* const key, int& outl )
{
   DEBUG4(cerr << "CryptHandler::decryptBuffer( " << len << " )" << endl;);
   
   EVP_CIPHER_CTX* cCtx;
   const EVP_CIPHER *c;

   const char* iv = "ahfy573E"; // Must be same in enc and dec
                               // not used with all cryptos (not in rc4)

   cCtx = ( EVP_CIPHER_CTX *)OPENSSL_malloc(sizeof(EVP_CIPHER_CTX));

   c = EVP_rc4();

   EVP_CIPHER_CTX_init( cCtx );

   EVP_DecryptInit( cCtx, c, (unsigned char*)key, (unsigned char*)iv );
   
   outl = len; // TODO: true when using symetric cipher else perhaps use:
              //EVP_DECODE_LENGTH( len )*2;
   ScopedArray<byte> out( new byte[ outl ] ); 

   if ( ! EVP_DecryptUpdate( cCtx, out.get(), &outl, 
                             (unsigned char*)inBuff, len ) )
   {
      MC2WARNING( "CryptHandler::decryptBuffer DecryptUpdate falied" );
      return NULL;
   }

   int finall = 0;
   // The padding if any 
   if ( ! EVP_DecryptFinal( cCtx, out.get() + outl, &finall ) ) {
      MC2WARNING( "CryptHandler::decryptBuffer DecryptFinal falied" ) ;
      return NULL;
   }

   outl += finall;
   
   EVP_CIPHER_CTX_cleanup( cCtx );

   return out.release();
}


byte* 
CryptHandler::compressBuffer( const byte* const inBuff, uint32 len,
                              int& outl )
{
   byte* out = NULL;
   COMP_METHOD* comp = NULL;
#ifdef ZLIB
   comp = COMP_zlib();
#endif
   if ( comp != NULL ) {
      COMP_CTX* ctx;

      ctx = COMP_CTX_new( comp );
      
      outl = len + 1; // Max size if clean
      out = new byte[ outl ];
      
      outl = COMP_compress_block( ctx, out, outl, (byte*)inBuff, len );
      
      COMP_CTX_free( ctx );
      
      return out;
   } else {
      // Just copy inBuff, but add extra 0 byte first to indicate clear
      MC2WARNING( 
         "CryptHandler::compressBuffer no compression available!" );
      outl = len + 1;
      out = new byte[ outl ];
      out[ 0 ] = 0; // Indicate clear compression
      memcpy( out + 1, inBuff, len );
      return out;
   }
}


byte* 
CryptHandler::uncompressBuffer( const byte* const inBuff, uint32 len,
                                int& outl )
{
   ScopedArray<byte> out;
   COMP_METHOD* comp = NULL;
#ifdef ZLIB
   comp = COMP_zlib();
#endif
   if ( comp != NULL ) {
      COMP_CTX* ctx;

      ctx = COMP_CTX_new( comp );
      
      outl = len*2 + 1024; // Just a start value
      out.reset( new byte[ outl ] );
      int used = 0;
      bool expanded = false;

      while ( !expanded && (outl < (MAX_INT32/4)) ) {
         used = COMP_expand_block( ctx, out.get(), outl, 
                                   (byte*)inBuff, len );

         if ( used != -1 ) {
            expanded = true;
         } else if ( outl < (MAX_INT32/8) ) {
            // Allocate more
            outl *= 2;
            out.reset( new byte[ outl ] );
         } else {
            // No more allocations
            outl = MAX_INT32/4; 
         }
      }

      if ( expanded ) {
         outl = used;
      } else {
         // Proberbly not a compressed buffer
         out.reset( NULL );
         outl = 0;
      }
      
      COMP_CTX_free( ctx );

      return out.release();
   } else {
      // Just copy inBuff, but check if first byte is 0 then it should 
      // be clear
      if ( inBuff[ 0 ] == 0 ) {
         outl = len - 1;
         out.reset( new byte[ outl ] );
         memcpy( out.get(), inBuff + 1, outl );
         return out.release();
      } else {
         MC2WARNING( 
            "CryptHandler::uncompressBuffer no compression available!" );
         return NULL;
      }
   }  
}


#endif
