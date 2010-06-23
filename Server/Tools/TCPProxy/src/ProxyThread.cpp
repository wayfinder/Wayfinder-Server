/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ProxyThread.h"
#include "TCPSocket.h"
#include "SelectableSelector.h"
#include "STLStringUtility.h"
#include "NetUtility.h"
#include "LogBuffer.h"

ProxyThread::ProxyThread( TCPSocket* sock, const MC2String& host, uint16 port )
      : m_inSocket( sock ), m_outSocket( NULL ), 
        m_selector( new SelectableSelector() ), m_host( host ),
        m_port( port ), m_inShouldClose( 0 ), m_outShouldClose( 0 )
{
}

ProxyThread::~ProxyThread() {
   delete m_inSocket;
   delete m_outSocket;
   delete m_selector;
}

void
ProxyThread::addOutBytes( vector<byte>& outBuffer, 
                          const byte* startByte, const byte* endByte )
{
   outBuffer.insert( outBuffer.end(), startByte, endByte );
}

bool
ProxyThread::sendAndGet( TCPSocket* sock, bool& sockRead, bool& sockWrite, 
                         vector<byte>& outBuffer, vector<byte>& inBuffer,
                         TCPSocket* other, int& otherShouldClose,
                         bool& otherSockWrite, 
                         int& thisShouldClose, const MC2String& sockName,
                         const MC2String& otherSockName ) {
   mc2dbg4 << " outBuffer " << outBuffer.size() << " inBuffer " 
          << inBuffer.size() << " otherShouldClose " << otherShouldClose 
          << " thisshouldClose " << thisShouldClose << endl;
   // Get some
   const uint32 buffSize = 4096;
   byte buff[ buffSize ];
   ssize_t nbrOfbytes;
   bool res = true;

   nbrOfbytes = sock->read( buff, buffSize );
   mc2dbg4 << "read nbrOfbytes " << nbrOfbytes << endl;

   if ( nbrOfbytes > 0 ) {
      //outBuffer.insert( outBuffer.end(), buff, buff + nbrOfbytes );
      addOutBytes( outBuffer, buff, buff + nbrOfbytes );
      mc2log << "read " << nbrOfbytes << " bytes from " << sockName
             << endl;
   } else if ( nbrOfbytes == 0 ) {
      // Closed -> close other
      if ( otherShouldClose == 0 ) {
         otherShouldClose = 1;
      }
      mc2dbg4 << " closed on read otherShouldClose" << endl;
   } else if ( nbrOfbytes == -3 ) {
      // EAGAIN, no bytes now
      mc2dbg4 << " no read bytes " << endl;
   } else {
      // Error!
      mc2log << "Read error on socket! " << sockName << " error: "
             << strerror( errno ) << endl;
      
      res = false;
   }

   if ( res ) {
      if ( !inBuffer.empty() && thisShouldClose != -1 ) {
         // Write some
         nbrOfbytes = sock->write( &inBuffer.front(), inBuffer.size() );
         mc2dbg4 << "write nbrOfbytes " << nbrOfbytes << endl;
         if ( nbrOfbytes > 0 ) {
            inBuffer.erase( inBuffer.begin(), inBuffer.begin() + nbrOfbytes );
            mc2dbg4 << "wrote " << nbrOfbytes << " bytes " 
                   << inBuffer.size() << " bytes left" << endl;
            mc2log << "wrote " << nbrOfbytes << " bytes to " << sockName
                   << endl;
         } else if ( nbrOfbytes == 0 ) {
            // Closed -> close other
            if ( otherShouldClose == 0 ) {
               otherShouldClose = 1;
            }
            // And drop the remaining bytes to write as we can't write them
            if ( !inBuffer.empty() ) {
               mc2log << warn << "Socket " << sockName << " closed can't send "
                      << "remaining " << inBuffer.size() << " bytes" << endl;
            }
            inBuffer.clear();
            mc2dbg4 << " closed on write otherShouldClose" << endl;
         } else if ( nbrOfbytes == -3 ) {
            // EAGAIN, can't write bytes now
         } else {
            // Error!
            mc2log << "Write error on socket! " << sockName << " error: " 
                   << strerror( errno ) << endl;
            res = false;
         }
      }
   }

   if ( !inBuffer.empty() ) {
      sockWrite = true;
   }
   if ( !outBuffer.empty() ) {
      otherSockWrite = true;
   }

   if ( (otherShouldClose == 1) && outBuffer.empty() ) {
      // Close it
      mc2dbg4 << " shuting down other" << endl;
      ::shutdown( other->getSelectable(), SHUT_WR );
      otherShouldClose = -1;
   }

   return res;
}

void
ProxyThread::selectable( TCPSocket* sock, bool& sockRead, bool& sockWrite,
                         SelectableSelector* selector ) {
   if ( sockRead || sockWrite ) {
      mc2dbg4 << "Adding selectable sockRead " << sockRead << " sockWrite "
             << sockWrite << endl;
      selector->addSelectable( sock, sockRead, sockWrite, false );
   } else {
      mc2dbg4 << "Removing selectable " << endl;
      selector->removeSelectable( sock, sockRead, sockWrite, false );
   }
}

void
ProxyThread::run() {
   m_outSocket = new TCPSocket();
   m_outSocket->open();
   m_outSocket->setBlocking( false );

   m_inSocket->setBlocking( false );
   MC2String prefix;
   MC2String inSockStr;
   MC2String outSockStr;
   {
      uint32 IP;
      uint16 port;
      m_inSocket->getPeerName( IP, port );
      m_inIPnPort = IPnPort( IP, port );
      prefix.append( "[" );
      prefix.append( NetUtility::ip2str( IP ) );
      prefix.append( ":" );
      STLStringUtility::uint2str( port, prefix );
      prefix.append( "] " );
      static_cast<LogBuffer*>(mc2log.rdbuf())->setPrefix( prefix.c_str() );
      inSockStr.append( NetUtility::ip2str( IP ) );
      inSockStr.append( ":" );
      STLStringUtility::uint2str( port, inSockStr );
      outSockStr.append( m_host );
      outSockStr.append( ":" );
      STLStringUtility::uint2str( m_port, outSockStr );
   }


   mc2log << "Connecting to " << m_host << ":" << m_port << endl;
   if ( !m_outSocket->connect( m_host.c_str(), m_port ) ) {
      mc2log << "Could not connect to " << m_host << ":" << m_port << endl;
      return;
   }
   bool connecting = true;

   while( !terminated ) {
      bool inSockRead = m_outShouldClose == 0 && !connecting;
      bool inSockWrite = false;
      bool outSockRead = m_inShouldClose == 0;
      bool outSockWrite = false;
      bool ok = true;

      // Connecting 
      if ( connecting /*m_outSocket->getState() == TCPSocket::OPEN*/ ) {
         // Continue with outSocket and wait with inSocket
         outSockWrite = true;
         outSockRead = false; // Await connection
      } else {
         // Do stuff with m_inSocket
         mc2dbg4 << "m_inSocket" << endl;
         ok = sendAndGet( m_inSocket, inSockRead, inSockWrite, 
                          m_outBuffer, m_inBuffer, m_outSocket,
                          m_outShouldClose, outSockWrite, m_inShouldClose,
                          inSockStr, outSockStr );
         if ( ok ) {
            // Do stuff with m_outSocket
            mc2dbg4 << "m_outSocket" << endl;
            ok = sendAndGet( m_outSocket, outSockRead, outSockWrite, 
                             m_inBuffer, m_outBuffer, m_inSocket,
                             m_inShouldClose, inSockWrite, m_outShouldClose,
                             outSockStr, inSockStr );
         }
      }

      if ( ok ) {
         SelectableSelector::selSet readReady;
         SelectableSelector::selSet writeReady;

         mc2dbg4 << "m_inSocket" << endl;
         selectable( m_inSocket, inSockRead, inSockWrite, m_selector );
         mc2dbg4 << "m_outSocket" << endl;
         selectable( m_outSocket, outSockRead, outSockWrite, m_selector );
         
         SelectableSelector::SelectStatus selStatus =
            m_selector->select( 5000000, readReady, writeReady );

         if ( selStatus == SelectableSelector::OK ) {
            // OK the sendAndGet in next loop reads the data

            if ( connecting && writeReady.find( m_outSocket ) != 
                 writeReady.end() ) {
               // Ready!
               mc2log << "Connected to " << m_host << ":" << m_port << endl;
               connecting = false;
            }
         } else if ( selStatus == SelectableSelector::TIMEOUT ) {
            // Ok try again
            mc2dbg4 << "Select timeout" << endl;
         } else if ( selStatus == SelectableSelector::ERROR ) {
            // Bad!
            mc2log << "Select returned error bailing out!" << endl;
            terminate();
         }
      } else {
         // Bad!
         mc2log << "Socket error bailing out!" << endl;
         terminate();
      }

      // Are we done?
      if ( m_outShouldClose == -1 && m_inShouldClose == -1 &&
           m_outBuffer.empty() && m_inBuffer.empty() ) {
         mc2log << "All data sent and both sockets closed ending." << endl;
         terminate();
      }
   } // while not terminated

   mc2log << "Done with connection from " << m_inIPnPort << endl;
}

