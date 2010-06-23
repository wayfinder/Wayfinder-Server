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

#include "StatisticsPacket.h"

#include "IPnPort.h"
#include "MapStatistics.h"
#include "MapSafeVector.h"

StatisticsPacket::StatisticsPacket(const IPnPort& address,
                                   const MapSafeVector& stats,
                                   uint64 optMem,
                                   uint64 maxMem,
                                   int queueLength,
                                   int32 rank)
                                   
      : Packet(MAX_PACKET_SIZE, STAT_PRIO, PACKETTYPE_STATISTICS,
               address.getIP(), address.getPort(), 0, 0)
{
   int pos = HEADER_SIZE;
   stats.saveAsMapStats(this, pos, optMem, maxMem, queueLength, rank);
   setLength(pos);
}

int
StatisticsPacket::getMapStatistics(MapStatistics& mapStats) const
{
   int pos = HEADER_SIZE;
   mapStats.load(this, pos);
   return pos;
}

// ____________________ SMS Statistics ____________________________

SMSStatisticsPacket::SMSStatisticsPacket(const IPnPort& address,
                                         const MapSafeVector& stats,
                                         int queueLength,
                                         uint16 numberOfPhones,
                                         char** phoneNumbers,
                                         uint16 numberOfServices,
                                         char** services)
      : StatisticsPacket(address, stats,
                         0, 0, queueLength, 0)
{
   // FIXME: Write SMSStats after the ordinary packet ends.   
   setSubType(PACKETTYPE_SMSSTATISTICS);
   // Not efficient, i know. Add stuff after ordinary packet start.
   MapStatistics mapStats;
   int position = getMapStatistics(mapStats);
   incWriteShort(position, numberOfPhones);
   incWriteShort(position, numberOfServices);
   for(int i=0; i < numberOfPhones; i++) {
      incWriteString(position, phoneNumbers[i]);
   }
   for(int j=0; j < numberOfServices; j++) {
      incWriteString(position, services[j]);
   }
   // Now set the length of the packet (rather important)
   setLength(position);
}

uint16
SMSStatisticsPacket::getNumberOfPhoneNumbers() const
{
   MapStatistics stats;
   int position = getMapStatistics(stats);
   return readShort(position);
}


uint16
SMSStatisticsPacket::getNumberOfServiceNames() const
{
   MapStatistics stats;
   int position = getMapStatistics(stats) + 2;   
   return readShort(position);
}


void
SMSStatisticsPacket::getPhonesAndServices(char *phoneNumbers[],
                                          char *serviceNames[]) const
{
   int numberOfServices = getNumberOfServiceNames();
   int numberOfPhones   = getNumberOfPhoneNumbers();

   // Start at stats-position
   MapStatistics stats;
   int position =  getMapStatistics(stats) + 4;

   // Read in the phoneNumbers.
   for ( int i=0; i < numberOfPhones; i++) {
      incReadString(position, phoneNumbers[i]);
   }
   // Read in the service names.
   for ( int i=0; i < numberOfServices; i++) {
      incReadString(position, serviceNames[i]);
   }
}
