/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2_CRYPTHANDLER_H
#define MC2_CRYPTHANDLER_H

#if defined(USE_SSL) || defined(USE_CRYPTO)

#include "config.h"
// OpenSSL crypto
#include <openssl/crypto.h>
// EVP interface to OpenSSL crypto
#include <openssl/evp.h>
// Compression
#include <openssl/comp.h>


/**
 *  Class for encrypting and decrypting.
 *
 * "This product includes software developed by the OpenSSL Project
 *  for use in the OpenSSL Toolkit. (http://www.openssl.org/)"
 *
 */
class CryptHandler {
   public:
      /**
       * Initializes libary.
       */
      static void initialize();


      /**
       * Encodes inBuff using key key and returns a new encrypted buffer.
       *
       * @param inBuff The buffer to encrypt.
       * @param len The length of inBuff.
       * @param key The key to encrypt with, must be 24 bytes long
       *            (192 bits) to make sure its not to short.
       * @param outl Set to the length of the returned byte vector,
       *             is not set if encrytion fails (returns NULL).
       * @return The encrypted buffer or NULL if error.
       */
      static byte* encryptBuffer( const byte* const inBuff, uint32 len,
                                  const char* const key, int& outl );

      
      /**
       * Decodes inBuff using key key and returns a new unencoded buffer.
       *
       * @param inBuff The buffer to decrypt.
       * @param len The length of inBuff.
       * @param key The key to decrypt with, must be 24 bytes long
       *            (192 bits) to make sure its not to short and be the 
       *            same that encrypted the data!.
       * @param outl Set to the length of the returned byte vector,
       *             is not set if decrytion fails (returns NULL).
       * @return The decrypted buffer or NULL if error.
       */
      static byte* decryptBuffer( const byte* const inBuff, uint32 len,
                                  const char* const key, int& outl );
   

      /**
       * Compress a buffer.
       * 
       * @param inBuff The buffer to compress.
       * @param len The length of inBuff.
       * @param outl Set to the length of the returned byte vector,
       *             is not set if compression fails (returns NULL).
       * @return The compressed buffer or NULL if error.
       */
      static byte* compressBuffer( const byte* const inBuff, uint32 len,
                                   int& outl );

      
      /**
       * Uncompress a buffer compressed with compressBuffer.
       * 
       * @param inBuff The buffer to uncompress.
       * @param len The length of inBuff.
       * @param outl Set to the length of the returned byte vector,
       *             is not set if uncompression fails (returns NULL).
       * @return The uncompressed buffer or NULL if error.
       */
      static byte* uncompressBuffer( const byte* const inBuff, uint32 len,
                                     int& outl );

   private:
      /**
       * Private constructor to avoid instances of this class.
       */
      CryptHandler();


};

#endif // defined(USE_SSL) || defined(USE_CRYPTO)

#endif // MC2_CRYPTHANDLER_H
