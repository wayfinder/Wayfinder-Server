/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include "CommunicationProcessor.h"
#include "Packet.h"
#include "SendDataPacket.h"
#include "SendDataInterfaceRequest.h"
#include "TimeUtility.h"
#include "PeriodicPacket.h"
#include "NewStrDup.h"
#include "STLStringUtility.h"
#include "StringTable.h"

const uint32 defaultQueueTimeout = 10;/*seconds*/
const uint32 defaultBackoffFactor = 10;/*to multiply queueTimeou with*/
const uint32 maxQueueTimeout = 36000/*10h*/;

CommunicationProcessor::SDIRHolder::SDIRHolder( SendDataInterfaceRequest* s )
      : m_s( s ), m_queueStartTime( TimeUtility::getRealTime() )
{
}

uint32
CommunicationProcessor::SDIRHolder::timeOutEnd() const {
   return m_queueStartTime + m_s->getQueueTimeout();
}

SendDataInterfaceRequest*
CommunicationProcessor::SDIRHolder::getS() {
   return m_s;
}

bool
CommunicationProcessor::SDIRHolder::operator < ( const SDIRHolder& o ) const {
   return timeOutEnd() < o.timeOutEnd();
}


CommunicationProcessor::CommunicationProcessor( MapSafeVector* loadedMaps )
      : Processor( loadedMaps ),
        m_io( new SelectableInterfaceIO( this ) )
#ifdef USE_SSL
      , m_ctx( NULL )
#endif
{
#ifdef USE_SSL
   const char pathSep = '/';
   const char* path = "./";
   const char* certFile = "httpd.pem";
   char* certFilename = NewStrDup::newStrDup( certFile );
   char* orgCertFilename = certFilename;
   char* pos = strrchr( certFilename, pathSep );
   if ( pos != NULL ) {
      // Split certFile in path and file
      *pos = '\0';
      certFilename = ++pos;
      path = certFile; // original filename
   }
   m_ctx.reset( SSLSocket::makeNewCTX( SSLSocket::SSL_SOCKET_CLIENTS,
                                       path,
                                       certFilename,
                                       certFilename,
                                       certFilename,
                                       NULL) );
   delete [] orgCertFilename;
#endif
}

CommunicationProcessor::~CommunicationProcessor() {
   m_io->shutdownStarts();
#ifdef USE_SSL
   SSLCleaner();
#endif
}

int
CommunicationProcessor::putInterfaceRequest( InterfaceRequest*& ireq ) {
   SendDataInterfaceRequest* s = static_cast< SendDataInterfaceRequest* > (
      ireq );
#ifdef DEBUG_LEVEL_2
   vector<byte> replyBytes = s->getReplyBytes();
   replyBytes.push_back( '\0' );
   const char* replyData = reinterpret_cast<char*>( &replyBytes.front() );
   vector<byte> requestBytes;
   requestBytes.insert( 
      requestBytes.end(), s->getRequestBuffer(),
      s->getRequestBuffer() + s->getRequestSize() );
   requestBytes.push_back( '\0' );
   const char* requestData = reinterpret_cast<char*>( &requestBytes.front() );
   mc2dbg << "[CMMP] got reply in state: " 
          << InterfaceRequest::getStateAsString( s->getState() ) 
          << " Data: " << endl 
          << replyData << endl
          << "expected: "  << s->getExpectedReplyData() << endl
          << requestData << endl;
#endif

   if ( ireq->getState() != InterfaceRequest::Done ) {
      // Try again, after timeout.
      // Close socket here before putting request in queue.
      if ( s->getReqSock()->getState() != TCPSocket::CLOSED &&
           s->getReqSock()->getState() != TCPSocket::UNKNOWN ) {
         s->getReqSock()->close();
      }
      m_queue.insert( SDIRHolder( s ) );
      ireq = NULL;
   } else {
      mc2log << info << s->getLogPrefix() << " DONE! " << endl;
   }

   delete ireq;
   ireq = NULL;
   return 0;
}

void
CommunicationProcessor::handleDoneInterfaceRequest( InterfaceRequest* ireply )
{
   putInterfaceRequest( ireply );
}

Packet*
CommunicationProcessor::handleRequestPacket( const RequestPacket& request,
                                             char* packetInfo )
{
   ReplyPacket* reply = NULL;

   mc2dbg4 << "CMMP::handleRequest() going to handle packet" << endl;

   switch ( request.getSubType() ) {
      case Packet::PACKETTYPE_SEND_DATA_REQUEST :
         reply = handleSendDataRequest( 
            static_cast< const SendDataRequestPacket* > ( &request ), 
            packetInfo ); 
         break;
      case Packet::PACKETTYPE_PERIODIC_REQUEST : {
         uint32 now = TimeUtility::getRealTime();
         SDIRQueue::iterator it = m_queue.begin();
         while ( it != m_queue.end() && it->timeOutEnd() <= now ) {
            SendDataInterfaceRequest* s = 
               const_cast< SDIRHolder& >( *it ).getS();
            s->reset();
            s->setQueueTimeout( 
               MIN( s->getQueueTimeout() * defaultBackoffFactor, 
                    maxQueueTimeout ) );
            m_io->putInterfaceRequest( s );
            m_queue.erase( it++ );
         }
      }  break;
      default:
         mc2log << warn << "[CP]: Unknown packet "
                << request.getSubTypeAsString() << endl;
         break;
   }
   
   return reply;
}

int CommunicationProcessor::getCurrentStatus() {
   return 0;
}


SendDataReplyPacket*
CommunicationProcessor::handleSendDataRequest( 
   const SendDataRequestPacket* req, char* packetInfo )
{
   SendDataReplyPacket* reply = new SendDataReplyPacket( 
      req, StringTable::OK );

   MC2String reqData( reinterpret_cast<const char*>( req->getData() ),
                      req->getDataLength() );
   if ( req->getDataType() == SendDataRequestPacket::http ) {
      // Add Content-Length and host
      MC2String::size_type findPos = reqData.find( "\r\n\r\n" );
      if ( findPos != MC2String::npos ) {
         findPos += 4; // CRLFCRLF
         uint32 contentLength = reqData.size() - findPos;
         MC2String addHeaders;
         addHeaders += "\r\n";
         addHeaders += "Content-Length: ";
         STLStringUtility::uint2str( contentLength, addHeaders );
         addHeaders += "\r\n";
         addHeaders += "Host: ";
         addHeaders += req->getPeer().getHost();
         addHeaders += "\r\n";
         // EOH
         addHeaders += "\r\n";
         STLStringUtility::replaceString( reqData, "\r\n\r\n", addHeaders );
#ifdef DEBUG_LEVEL_2
         mc2dbg << "Request " << reqData << endl;
         Utility::hexDump( mc2log, (byte*)( 
                              reqData.data() ),
                           reqData.size(), "req " );
#endif
      }
   }

   SendDataInterfaceRequest* s = new SendDataInterfaceRequest( 
      req->getPeer(), 
      reinterpret_cast<const byte*>( reqData.data() ),
      reqData.size(),
      req->getExpectedReplyData(),
#ifdef USE_SSL
      m_ctx.get(),
#endif
      defaultQueueTimeout );
   mc2log << info << s->getLogPrefix() << " starting." << endl;

   m_io->putInterfaceRequest( s );

   return reply;
}
