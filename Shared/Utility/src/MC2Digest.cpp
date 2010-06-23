/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MC2Digest.h"
#include "StringUtility.h"
#include "openssl/evp.h"

struct MC2Digest::Impl {
   EVP_MD_CTX ctx;
   const EVP_MD* md;
};


MC2Digest::MC2Digest( const MC2String& digestName ) 
      : m_impl( new Impl() )
{
   OpenSSL_add_all_digests();
   EVP_MD_CTX_init( &m_impl->ctx );
   m_impl->md = EVP_get_digestbyname( digestName.c_str() );
   if ( m_impl->md == NULL ) {
      mc2log << fatal << "MC2Digest digest " << digestName << " not found."
             << endl;
      MC2_ASSERT( m_impl->md != NULL );
   }
}

MC2Digest::~MC2Digest() {
   EVP_MD_CTX_cleanup( &m_impl->ctx );
}

bool
MC2Digest::digest( const MC2String& data, MC2String& digest ) {
   if ( data.empty() ) {
      mc2log << warn << "MC2Digest::digest empty string" << endl;
      return false;
   }

   // Set up for digest
   if ( EVP_DigestInit_ex( &m_impl->ctx, m_impl->md, NULL ) != 1 ) {
      mc2log << warn << "MC2Digest::digest EVP_DigestInit_ex failed " << endl;
      return false;
   }

   // Make the digest
   if ( EVP_DigestUpdate( &m_impl->ctx, data.c_str(), data.size() ) != 1 ) {
      mc2log << warn << "MC2Digest::digest EVP_DigestUpdate failed " << endl;
      return false;
   }

   unsigned char digestBuff[ EVP_MAX_MD_SIZE ];
   unsigned int digestLength = 0;
   // And finalize digest, and get length
   if ( EVP_DigestFinal_ex( &m_impl->ctx, digestBuff, &digestLength) != 1 ) {
      mc2log << warn << "MC2Digest::digest EVP_DigestFinal_ex failed " << endl;
      return false;
   }

   // Make text of the digest
   digest = StringUtility::base64Encode( 
      MC2String( reinterpret_cast< const char* >( digestBuff ), 
                 digestLength ) );
   
   return true;
}
