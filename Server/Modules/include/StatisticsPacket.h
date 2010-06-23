/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STATISTICS_PACKET_H
#define STATISTICS_PACKET_H

#include "config.h"

#include "Packet.h"

class MapStatistics;
class MapSafeVector;
class IPnPort;

#define MAX_STAT_LENGTH      MAX_PACKET_SIZE
#define STAT_PRIO             0

/** 
 *  Class for handling the statistics packets.
 *  Really for mapstatustucs.
 */
class StatisticsPacket : public Packet {
public:

   /**
    *   Constructor for StatisticsPacket.
    *   @param address     Sender of packet.
    *   @param stats       The MapSafeVector which contains info about maps.
    *   @param optMem      Optimal memory for the module.
    *   @param maxMem      Maximum memory for the module.
    *   @param queueLength Length of queue to JobThread.
    *   @param rank        Rank of the module.
    */
   StatisticsPacket(const IPnPort& address,
                    const MapSafeVector& stats,
                    uint64 optMem,
                    uint64 maxMem,
                    int queueLength,
                    int32 rank);

   /**
    *   Loads the map statistics from the packet.
    *   @return The position
    */
   int getMapStatistics(MapStatistics& mapStats) const;
   
}; 

 /** 
   *  Class for handling the statistics packets for SMS Modules.
   * 
   *  In addition to the normal header this packet contains
   *  \begin{tabular}{lll}
   *     Pos & Size    & \\ \hline
   *     HEADER_SIZE  & 4 bytes & Statistics
   *     +4           & 4 bytes & nbrMaps ( always == 0 )
   *     +8           & 2 bytes & Number of phonenumbers for 
   *                              this module = X.
   *     +10          & 2 bytes & Number of servicenames for 
   *                              this module = Y.
   *     +12          & string*X  & The phone numbers for this module.
   *     ??           & string*Y  & The service names for this module.
   *  \end{tabular}
   */
class SMSStatisticsPacket : public StatisticsPacket {
public:

   /**
    *   Creates a new SMSStatisticsPacket.
    *   @param address The ip and port of the available module
    *                  sending the packet.
    *   @param stats   The MapSafeVector.
    *   @param numberOfPhones The number of phonenumbers that the SMSModule
    *                         can send SM:s to.
    *   @param phoneNumbers   The phonenumbers that the SMSModule can send
    *                         short messages to.
    *   @param numberOfServices The number of services for the SMSModule.
    *   @param services         The services of the SMSModule. If a service
    *                           name is here the leader will send SM:s that
    *                           have that name for phonenumber to this module.
    */
   SMSStatisticsPacket(const IPnPort& address,
                       const MapSafeVector& stats,
                       int queueLength,
                       uint16 numberOfPhones,
                       char** phoneNumbers,
                       uint16 numberOfServices,
                       char** services);
            
   /**
    *   Returns the number of phonenumbers for the SMSStatisticsPacket.
    *   @return The number of the phoneNumbers.
    */
   uint16 getNumberOfPhoneNumbers() const;

   /**
    *   Returns the number of services of the SMSStatisticsPacket.
    *   @return The number of services.                       
    */
   uint16 getNumberOfServiceNames() const;

   /**
    *   Use this function to read out the service names. First call
    *   getNumberOfServiceNames and allocate a buffer to store the pointers
    *   in. NB: No copies are made. The pointers point into the packet.
    *   @param phoneNumbers An array to put the phoneNumberpointers into.
    *   @param serviceNames An array to put the pointers to the service
    *                       names into.
    *   @return The number of services 
    */
   void getPhonesAndServices(char *phoneNumbers[],
                             char *serviceNames[]) const;
};



#endif
