/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "HttpProxyThread.h"
#include "File.h"
#include "StringUtility.h"

HttpProxyThread::HttpProxyThread( TCPSocket* sock,
                                  const MC2String& host,
                                  uint16 port,
                                  const MC2String& msisdnFile,
                                  const MC2String& hmccFile )
      : ProxyThread( sock, host, port ),
      m_msisdn( msisdnFile ),
      m_hmcc( hmccFile )
{
}

HttpProxyThread::~HttpProxyThread() {
}

void
HttpProxyThread::addOutBytes( vector<byte>& outBuffer, 
                              const byte* startByte, const byte* endByte )
{
   // Add these bytes too
   size_t startStoreSize = m_store.size();
   m_store.insert( m_store.end(), startByte, endByte );

   MC2String byteStr( reinterpret_cast<const char*>( &m_store.front() ), 
                      m_store.size() );
   bool inserted = false;
   size_t findPos = byteStr.find( "HTTP/1." );
   if ( findPos != MC2String::npos ) {
      size_t insertPos = byteStr.find( "\n", findPos );
      if ( insertPos != MC2String::npos ) {
         // Insert the lines!!!
         vector<byte> msisdnBuff;
         File::readFile( m_msisdn.c_str(), msisdnBuff );
         MC2String msisdnStr( reinterpret_cast<const char*>( 
                                 &msisdnBuff.front() ),
                              msisdnBuff.size() );
         msisdnStr = StringUtility::trimStartEnd( msisdnStr );
         byteStr.insert( insertPos + 1, 
                         MC2String( "x-sdp-msisdn: " ) + msisdnStr + "\r\n" );

         vector<byte> hmccBuff;
         File::readFile( m_hmcc.c_str(), hmccBuff );
         MC2String hmccStr( reinterpret_cast<const char*>( 
                                 &hmccBuff.front() ),
                            hmccBuff.size() );
         hmccStr = StringUtility::trimStartEnd( hmccStr );
         byteStr.insert( insertPos + 1, 
                         MC2String( "x-up-sgsn-hmccmnc: " ) + hmccStr+"\r\n" );
         inserted = true;
      }
   }
   if ( inserted ) {
      // Clear the kept bytes
      m_store.clear();
      startByte = reinterpret_cast<const byte*>( byteStr.data() + 
                                                 startStoreSize );
      endByte = startByte + (byteStr.size() - startStoreSize);
   } else {
      // Remove the stored bytes at the start of this function, they where of
      // no use
      m_store.erase( m_store.begin(), m_store.begin() + startStoreSize );
   }

   outBuffer.insert( outBuffer.end(), startByte, endByte );
}
