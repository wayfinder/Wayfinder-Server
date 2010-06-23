/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCEPUSHPACKET_H
#define DISTURBANCEPUSHPACKET_H

#define PUSH_REQUEST_PRIO (DEFAULT_PACKET_PRIO-1)

#include "PushPacket.h"
#include "DisturbanceElement.h"
#include "DisturbancePacketUtility.h"
#include <map>

/**
 *    Creates a DisturbancePushPacket.
 *
 */
class DisturbancePushPacket : public PushPacket
{
  public:
   
   /**
    *  Constructor
    *  @param mapID The mapID for the disturbance.
    *  @param serviceID The serviceID for the PushPacket.
    *  @param timeStamp The time stamp for the PushPacket.
    *  @param distMap The multimap with the disturbances.
    *  @param removeDisturbance True if the disturbances are going to be
    *                           removed.
    */
   DisturbancePushPacket(uint32 mapID,
                         uint32 serviceID,
                         uint32 timeStamp,
                         const map<uint32, const DisturbanceElement*>& distMap,
                         bool removeDisturbance,
                         bool removeAll = false);

   /**
    *  Destructor
    *
    */
   virtual ~DisturbancePushPacket();

   /**
    *  Returns the disturbances.
    *  @param distVect  The vector with the disturbances.
    *  @param removeDisturbance True if the disturbance is going to be
    *                           removed.
    */
   void  getDisturbances(map<uint32, DisturbanceElement*> &distMap,
                         bool& removeDisturbance,
                         bool& removeAll) const;

};

#endif

