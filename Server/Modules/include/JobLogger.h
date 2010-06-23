/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef JOBLOGGER_H
#define JOBLOGGER_H

#include "FixedSizeString.h"
#include "JobReply.h"
#include "NotCopyable.h"

class Packet;
class PacketQueue;

/**
 * Takes care of the job logging for packets and replies.
 * Usage:
 * \code
 *
 * JobLogger logger( 1, incomingQueue );
 * // start a scope with new logging
 * {
 *    JobLogger::LogScope scope( logger, incomingPacket, reply );
 *    // log prefix here
 *    // do stuff
 * }
 * // no log prefix here.
 * \endcode
 */
class JobLogger: private NotCopyable {
public:
   /**
    * Helper class for log scope, makes sure we do not
    * get any inconsistent state with the JobLogger.
    * It also makes sure we can not call updateProcessLog unless
    * we are in a logging scope.
    */
   class LogScope {
   public:
      LogScope( JobLogger& log,
                const Packet* packet,
                JobReply& reply ):
         m_log( log ),
         m_disabled( false ) {
         m_log.startLog( packet, reply );
      }
      ~LogScope() {
         if ( ! m_disabled ) {
            m_log.endLog();
         }

      }
      /// @see JobLogger::updateProcessLog
      void updateLogProcess() {
         m_log.updateLogProcess();
      }

      /// Disable logging.
      void disable() {
         m_disabled = true;
      }

   private:
      /// The logger to use in the scope.
      JobLogger& m_log;
      /// Whether the logging was disabled during scope.
      bool m_disabled;
   };

   /**
    * @param jobCourierID The id of the courier.
    * @param receiveQueue Incoming queue for packets.
    */
   JobLogger( uint32 jobCourierID,
              const PacketQueue& receiveQueue );

   ~JobLogger();

   /**
    * Number of microseconds since logging was started, @see LogScope
    * @return microseconds since logging was started.
    */
   uint32 getProcessingTime() const;

   /**
    * @return Current packet info data.
    */
   FixedSizeString& getPacketInfoString() {
      return m_packetInfo;
   }

private:
   /**
    * So LogScope can make sure we keep this instance in a
    * consistent state.
    */
   friend class JobLogger::LogScope;

   /// Start logging, i.e set special prefix.
   void startLog( const Packet* packet, JobReply& reply );

   /// Update processing time.
   void updateLogProcess();

   /// End logging, i.e set default prefix.
   void endLog();

   /// Set current log prefix from packet info.
   void setLogPrefix( const Packet* packet );

   /// Start time in microseconds.
   uint32 m_startTime;
   /// Start time in milliseconds.
   uint32 m_startTimeMillis;
   /// Processor time used by the program.
   clock_t m_startClock;
   /// Packet arrival time.
   int m_arrivalTime;
   /// JobCourier ID.
   uint32 m_id;
   /// Buffer for packet info during logging
   FixedSizeString m_packetInfo;
   /// Buffer for log prefix during log scope.
   FixedSizeString m_logPrefix;
   /// Current reply in log scope.
   JobReply* m_reply;
   /// Incoming packets.
   const PacketQueue& m_receiveQueue;
};

#endif // JOBLOGGER_H
