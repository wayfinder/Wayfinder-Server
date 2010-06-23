/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef HTTP_PROXY_THREAD_H
#define HTTP_PROXY_THREAD_H


#include "config.h"
#include "ISABThread.h"
#include "ProxyThread.h"
#include <vector>

class TCPSocket;


/**
 * Sun class to ProxyThread for adding http headers.
 *
 */
class HttpProxyThread : public ProxyThread
{
public:
   /**
    * The debug http proxy.
    */
   HttpProxyThread( TCPSocket* sock, const MC2String& host, uint16 port,
                    const MC2String& msisdnFile, const MC2String& hmccFile );

   /**
    *
    */
   virtual ~HttpProxyThread();

protected:
   /**
    * Called
    * Default is to append the startByte to endByte range to outBuffer.
    *
    * @param outBuffer The buffer to add the bytes to.
    * @param startByte The first byte in the range to add.
    * @param endByte The byte after the last in the range.
    */
   virtual void addOutBytes( vector<byte>& outBuffer, 
                             const byte* startByte, const byte* endByte );

private:
   /**
    * The temporaryly stored bytes.
    */
   vector<byte> m_store;

   /// The filepath where the MSISDN is found.
   MC2String m_msisdn;
   /// The filepath where the home mcc is found.
   MC2String m_hmcc;
};

#endif // HTTP_PROXY_THREAD_H

