/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PROXYTHREAD_H
#define PROXYTHREAD_H


#include "config.h"
#include "ISABThread.h"
#include "IPnPort.h"
#include <vector>

class SelectableSelector;
class TCPSocket;


/**
 * Class for proxying a connection to a host:port.
 *
 */
class ProxyThread : public ISABThread 
{
public:
   /**
    *
    */
   ProxyThread( TCPSocket* sock, const MC2String& host, uint16 port );

   /**
    *
    */
   virtual ~ProxyThread();

   /**
    *  Start the thread and begin forwarding messages.
    */
   virtual void run();

protected:
   /**
    * Called when outgoing bytes, in any direction, should be added to
    * outBuffer.
    * Default is to append the range startByte to endByte to outBuffer.
    *
    * @param outBuffer The buffer to add the bytes to.
    * @param startByte The first byte in the range to add.
    * @param endByte The byte after the last in the range.
    */
   virtual void addOutBytes( vector<byte>& outBuffer, 
                             const byte* startByte, const byte* endByte );

  private:
   /**
    * Get and send bytes on socket.
    */
   bool sendAndGet( TCPSocket* sock, bool& sockRead, bool& sockWrite, 
                    vector<byte>& outBuffer, vector<byte>& inBuffer,
                    TCPSocket* other, int& otherShouldClose,
                    bool& otherSockWrite, 
                    int& thisShouldClose, const MC2String& sockName,
                    const MC2String& otherSockName );

   /**
    * Add/remove sokcet from selector.
    */
   void selectable( TCPSocket* sock, bool& sockRead, bool& sockWrite,
                    SelectableSelector* selector );

   /**
    * The incoming socket.
    */
   TCPSocket* m_inSocket;

   /**
    * The ourgoing socket.
    */
   TCPSocket* m_outSocket;

   /**
    * Selector to select on sockets.
    */
   SelectableSelector* m_selector;
   
   /**
    * Host to connect to.
    */
   MC2String m_host;
   
   /**
    * Port to connect to.
    */
   uint16 m_port;
   
   /**
    * Send buffer to outSocket.
    */
   vector<byte> m_outBuffer;

   /**
    * Send buffer to inSocket.
    */
   vector<byte> m_inBuffer;

   /**
    * If inSocket should be closed, outSocket closed, after sending all the 
    * bytes in buffer.
    */
   int m_inShouldClose;

   /**
    * If outSocket should be closed, inSocket closed, after sending all the 
    * bytes in buffer.
    */
   int m_outShouldClose;

   /**
    * The IP and port of the incoming socket.
    */
   IPnPort m_inIPnPort;
};

#endif // PROXYTHREAD_H

