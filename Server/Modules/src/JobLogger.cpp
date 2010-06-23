/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "JobLogger.h"

#include "Processor.h"
#include "LogBuffer.h"

#include "TimeUtility.h"
#include "Packet.h"
#include "MakeInfoString.h"

JobLogger::JobLogger( uint32 id,
                      const PacketQueue& receiveQueue ):
   m_startTime( 0 ),
   m_startTimeMillis( 0 ),
   m_startClock( 0 ),
   m_arrivalTime( 0 ),
   m_id( id ),
   m_packetInfo( Processor::c_maxPackInfo ),
   m_logPrefix( 32 ),
   m_receiveQueue( receiveQueue ) {
}

JobLogger::~JobLogger() {

}

void JobLogger::startLog( const Packet* packet,
                          JobReply& reply ) {
   setLogPrefix( packet );
   m_reply = &reply;

   // Zeroterm the extra string.
   m_packetInfo[ 0 ] = 0;

   m_startTime = TimeUtility::getCurrentMicroTime();
   m_startTimeMillis = TimeUtility::getCurrentTime();

   m_startClock = clock();
   m_arrivalTime = packet->getArrivalTime();

}

uint32 JobLogger::getProcessingTime() const {
   return TimeUtility::getCurrentMicroTime() - m_startTime;
}

void JobLogger::updateLogProcess() {
   // Get the time again.
   m_reply->m_processingTimeMillis = TimeUtility::getCurrentTime()
      - m_startTimeMillis;
   clock_t endClock = clock();

   m_reply->m_processorTimeMillis =
      ((float)(m_startClock) - endClock ) / float(CLOCKS_PER_SEC / 1000);

   if ( m_reply->m_reply != NULL ) {
      m_reply->m_reply->setDebInfo( m_reply->m_processingTimeMillis );
      m_reply->m_reply->setCPUTime( static_cast< uint32 >
                                    ( m_reply->m_processorTimeMillis ) );
   }

}

void JobLogger::endLog() {

   m_reply->m_timeSinceArrival = TimeUtility::getCurrentTime() - m_arrivalTime;

   JobThreadString::addReceiveQueueInfo( *m_reply, m_receiveQueue );
   JobThreadString::makeInfoString( m_packetInfo, *m_reply );

   mc2log << m_packetInfo << endl;

   setLogPrefix( NULL );
}

void JobLogger::setLogPrefix( const Packet* p )  {
   static const uint32 NUM_ORIGINATORS = 13;
   static const char origs[NUM_ORIGINATORS][3] =
           {"IN","SM","SS","HS","WS","NS","TE","MS","TS","MO","IS","XS","TR"};
   if (p == NULL) {
      static_cast<LogBuffer*>(mc2log.rdbuf())->setPrefix(NULL);
   } else {
      const char* originator = "Unknown";
      if ( (uint32)( p->getRequestOriginator() & 0xff ) < NUM_ORIGINATORS ) {
         originator = origs[ p->getRequestOriginator() & 0xff ];
      }

      if ( m_id == MAX_UINT32 ) {
         sprintf( m_logPrefix,
                  "[%s-%08X-%04X] ",
                  originator,
                  p->getRequestTimestamp(),
                  p->getRequestID() );
      } else {
         sprintf( m_logPrefix,
                  "[%s-%08X-%04X](%d) ",
                  originator,
                  p->getRequestTimestamp(),
                  p->getRequestID(),
                  m_id );
      }

      static_cast<LogBuffer*>(mc2log.rdbuf())->
         setPrefix( m_logPrefix.c_str() );
   }
}
