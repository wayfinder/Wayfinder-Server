/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ISABThread.h"
#include "config.h"
#include "TCPSocket.h"
#include "DatagramSocket.h"
#include "SocketReceiver.h"

/**
 *
 */
class ForwardThread : public ISABThread 
{
  public:
   /**
    *
    */
   ForwardThread(char* firstHost, uint16 firstPort,
                 char* secondHost, uint16 secondPort,
                 int maxBufSize = 65536);

   /**
    *
    */
   virtual ~ForwardThread();

   /**
    *  Connect a socket.
    */
   bool connect(char* host, uint16 port, TCPSocket* socket);
   
   /**
    *  Start the thread and begin forwarding messages.
    */
   void run();


  private:
   /**
    *  Private-protected default constructor
    */
   ForwardThread(){}

   /// Socket connecting to the first sender.
   TCPSocket* m_firstSocket;

   /// Socket connecting to the second sender.
   TCPSocket* m_secondSocket;

   /// SocketReceiver to select among the senders.
   SocketReceiver* m_socketReceiver;
   
   /// Name of the host of the first sender.
   char* m_firstHost;
   
   /// Port of the first sender.
   uint16 m_firstPort;
   
   /// Name of the host of the second sender.
   char* m_secondHost;
   
   /// Port of the second sender. 
   uint16 m_secondPort;

   /// Max amount of bytes for each forwarding.
   int m_maxBufSize;

   bool m_bothHosts;
   
   
};
