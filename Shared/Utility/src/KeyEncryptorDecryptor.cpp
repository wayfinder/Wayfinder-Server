/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "KeyEncryptorDecryptor.h"
#include "StringUtility.h"
#include "openssl/evp.h"

struct KeyEncryptorDecryptor::Impl {
   EVP_CIPHER_CTX ctx;
};


KeyEncryptorDecryptor::KeyEncryptorDecryptor()
      : m_impl( new Impl() )
{
   EVP_CIPHER_CTX_init( &m_impl->ctx );
}

KeyEncryptorDecryptor::~KeyEncryptorDecryptor() {
   EVP_CIPHER_CTX_cleanup( &m_impl->ctx );
}

bool
KeyEncryptorDecryptor::encrypt( const MC2String& inID, 
                                MC2String& outID ) {
   if ( inID.empty() ) {
      mc2log << warn << "KeyEncryptorDecryptor::encrypt empty string" 
             << endl;
      return false;
   }

   // Set up ctx for encryption
   if ( EVP_EncryptInit_ex( &m_impl->ctx, EVP_aes_256_cbc(), 
                            NULL/*use default engine*/, 
                            getKey(), getIV() ) 
        != 1 ) {
      mc2log << warn << "KeyEncryptorDecryptor::encrypt "
             << "EVP_EncryptInit_ex failed " << endl;
      return false;
   }

   // Make encryption buffer with space for an additional block
   int encLen = inID.size() + EVP_MAX_BLOCK_LENGTH;
   byte encBuff[ encLen ];

   // Encrypt the inID
   if ( EVP_EncryptUpdate( 
           &m_impl->ctx, encBuff, &encLen, 
           reinterpret_cast< const unsigned char* >( inID.c_str() ), 
           inID.size() ) != 1 ) {
      mc2log << warn << "KeyEncryptorDecryptor::encrypt "
             << "EVP_EncryptUpdate failed " << endl;
      return false;
   }

   // Add the padding, if any
   int paddingLen = 0;
   if( EVP_EncryptFinal_ex( &m_impl->ctx, encBuff + encLen, &paddingLen ) 
       != 1 ) {
      mc2log << warn << "KeyEncryptorDecryptor::encrypt "
             << "EVP_EncryptFinal_ex failed " << endl;
      return false;
   }

   // The paddingBytes are in encBuff
   encLen += paddingLen;

   // Make text of the encypted bytes, encBuff is input here
   outID = StringUtility::base64Encode( 
      MC2String( reinterpret_cast< const char* >( encBuff ), encLen ) );
   
   return true;
}

bool
KeyEncryptorDecryptor::decrypt( const MC2String& inID, 
                                MC2String& outID ) {
   // Make bytes of the text input
   byte encBuff[ inID.size() + 1 ];
   int encLen = StringUtility::base64Decode( inID.c_str(), encBuff );

   if ( inID.empty() || encLen <= 0 ) {
      mc2log << warn << "KeyEncryptorDecryptor::decrypt empty input" 
             << endl;
      return false;
   }

   // Set up ctx for decryption
   if ( EVP_DecryptInit_ex( &m_impl->ctx, EVP_aes_256_cbc(), 
                            NULL/*use default engine*/, getKey(), getIV() ) 
        != 1 ) {
      mc2log << warn << "KeyEncryptorDecryptor::decrypt "
             << "EVP_DecryptInit_ex failed " << endl;
      return false;
   }

   // Make decryptBuffer with space for an additional block
   int decLen = encLen + EVP_MAX_BLOCK_LENGTH;
   byte decBuff[ decLen ];

   // Decrypt the input bytes
   if ( EVP_DecryptUpdate( &m_impl->ctx, decBuff, &decLen, encBuff, encLen ) 
        != 1 ) {
      mc2log << warn << "KeyEncryptorDecryptor::decrypt "
             << "EVP_DecryptUpdate failed " << endl;
      return false;
   }

   // Decrypt the last block, if any
   int lastLen = 0;
   if( EVP_DecryptFinal_ex( &m_impl->ctx, decBuff + decLen, &lastLen ) 
       != 1 ) {
      mc2log << warn << "KeyEncryptorDecryptor::decrypt "
             << "EVP_DecryptFinal_ex failed " << endl;
      return false;
   }

   // The final bytes, if not a full block at end
   decLen += lastLen;
   
   outID.assign( reinterpret_cast< const char* >( decBuff ), decLen );
   return true;
}
