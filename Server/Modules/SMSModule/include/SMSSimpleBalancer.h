/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSSIMPLEBALANCER_H
#define SMSSIMPLEBALANCER_H

#include "config.h"

#include "SimpleBalancer.h"

class SMSStatisticsPacket;

class SMSSimpleBalancer : public SimpleBalancer {
public:
   /**
    *   Balancer for SMSModule.
    */
   SMSSimpleBalancer(const IPnPort& ownPort,
                     const MC2String& moduleName,
                     SMSStatisticsPacket* ownStatistics,
                     int nbrPhones,
                     int nbrServices,
                     char** phones,
                     char** services);

   /**
    *    @see Balancer.
    */
   bool updateStats(StatisticsPacket* packet, PacketSendList& );
   
   /**
    *    @see Balancer.
    */
   virtual bool getModulePackets(PacketSendList& packets,
                                 RequestPacket* req);

private:
   /**
    * Creates the module list.
    * @see SimpleBalancer::createModuleList
    */
   void createSMSModuleList( SMSStatisticsPacket* ownStatistics );

   /**
    *   The number of phone numbers.
    */
   int m_nbrPhones;

   /**
    *   The number of service names.
    */
   int m_nbrServices;

   /**
    *   The pointers to the phone numbers.
    */
   char** m_phones;

   /**
    *   The pointers to the service names.
    */
   char** m_services;
    
   
};


#endif
