/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include"ForwardThread.h"
#include"StringUtility.h"

#define MAX_NBR_RETRIES 5

ForwardThread::ForwardThread(char* firstHost, uint16 firstPort,
                             char* secondHost, uint16 secondPort,
                             int maxBufSize /*= 65536*/)
{
   m_firstHost      = StringUtility::newStrDup(firstHost);
   m_firstPort      = firstPort;
   m_secondHost     = StringUtility::newStrDup(secondHost);
   m_secondPort     = secondPort;

   m_maxBufSize     = maxBufSize;

   m_firstSocket    = new TCPSocket();
   m_secondSocket   = new TCPSocket();
   m_socketReceiver = new SocketReceiver();
   m_bothHosts      = false;
}

ForwardThread::~ForwardThread()
{
   delete m_firstSocket;
   delete m_secondSocket;
   delete m_socketReceiver;
   delete m_firstHost;
   delete m_secondHost;
}

bool 
ForwardThread::connect(char* host, uint16 port, TCPSocket* socket)
{
   // Connect to a port.
   int  nbrTries = 0;
   bool done     = false;
   while (!done && nbrTries++ < MAX_NBR_RETRIES - 1) {
      if (!socket->connect(host, port)) {
         mc2log << warn << "Couldn't connect to socket at " << host
                << ":" << port << ". Sleeping...ZZZZZzzzzzzz." <<endl;
         socket->close();
         sleep(200);
         socket->open();
      } else {
         done = true;
         m_socketReceiver->addTCPSocket(socket);
      }
   }
   if ( !done || nbrTries > MAX_NBR_RETRIES - 1) {
      mc2log << error << " Couldn't connect. "
             << "Check serial-port." << endl;
      return false;
   }   
   mc2dbg1 << " Connected to " << host << " : " << port
           << endl;
   return true;
}

void
ForwardThread::run()
{
   TCPSocket* t_rec;
   DatagramReceiver* d_rec;
   uint32 bufUint[m_maxBufSize/4+1];
   byte* buf = (byte*)bufUint;
   ssize_t nbrOfbytes;
   if(!connect(m_firstHost, m_firstPort, m_firstSocket)){
      mc2log << fatal << " Couldn't connect to first host, exiting " << endl;
      exit(1);
   }
   while(!terminated){
      if(m_socketReceiver->select(10000000, t_rec, d_rec)){
         if (!m_bothHosts) {
            if (t_rec == m_firstSocket){
               // Data from first
               nbrOfbytes = m_firstSocket->readMaxBytes(buf,
                                                        (ssize_t)m_maxBufSize);
               if (nbrOfbytes > 0){
                  if(connect( m_secondHost, m_secondPort, m_secondSocket))
                  {
                     m_bothHosts = true;
                     mc2log << info << "Connected to  second host. "
                            << "State change." << endl;
                     mc2dbg1 << "1->2 (" << nbrOfbytes << ")" << endl;
                     m_secondSocket->write(buf, nbrOfbytes);
                  }
               } else if (nbrOfbytes == -1){
                  
                  mc2log << fatal << "Lost first host, terminating." << endl;
                  exit(1);
               } else {
                  mc2log << error << "Got zero bytes from first!" << endl;
               }
            } else {
               mc2log << error << "Got other than first socket!" << endl;
            }
            
         } else {
            if (t_rec == m_firstSocket){
               // Data from first
               nbrOfbytes = m_firstSocket->
                  readMaxBytes(buf,(ssize_t)m_maxBufSize);
               if(nbrOfbytes > 0){
                  mc2dbg1 << "1->2 (" << nbrOfbytes << ")" << endl;
                  m_secondSocket->write(buf, nbrOfbytes);
               } else if (nbrOfbytes == -1){
                  mc2log << fatal << "Lost first host, terminating." << endl;
                  exit(1);
               } else {
                  mc2log << error << "Got zero bytes!" << endl;
               }
            } else if (t_rec == m_secondSocket){
               // Data from second.
               nbrOfbytes = m_secondSocket->
                  readMaxBytes(buf, (ssize_t)m_maxBufSize);
               if(nbrOfbytes > 0){
                  mc2dbg1 << "2->1 (" << nbrOfbytes << ")" << endl;
                  m_firstSocket->write(buf, nbrOfbytes);
                } else if (nbrOfbytes == -1){
                  mc2log << info << "Lost second host. State change" << endl;
                  m_bothHosts = false;
                  m_socketReceiver->removeTCPSocket(m_secondSocket);
                  m_secondSocket->close();
               } else {
                  mc2log << error << "Got zero bytes from second!" << endl;
               }  
            } else {
               mc2log << fatal << "Got strange socket!" << endl;
               exit (1);
            }
         }
         
      }
   }
   // Terminated
}

