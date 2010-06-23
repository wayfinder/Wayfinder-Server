/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SMSModuleList.h"
#include "SMSPacket.h"

SMSModuleList::
SMSModuleList() : ModuleList()
{
   
}

ModuleNotice*
SMSModuleList::getBestModule(Packet *p)
{
   if ( p != NULL) {
      if ( p->getSubType() == Packet::PACKETTYPE_SMSREQUEST ||
           p->getSubType() == Packet::PACKETTYPE_SMSLISTENREQUEST ) {
         bool decodingSuccess = false;
         char* compareNumber = NULL;
         if ( p->getSubType() == Packet::PACKETTYPE_SMSREQUEST ) {
            SMSSendRequestPacket* srp = (SMSSendRequestPacket*)p;
            // Decode the packet
            int encodingType = CONVERSION_NO;
            char* senderPhone = NULL;
            char* recipientPhone = NULL;
            int dataLength = 0;
            byte* data;
            decodingSuccess = srp->getPacketContents(encodingType, senderPhone,
                                                    recipientPhone, dataLength,
                                                    data);
            delete [] data;
            compareNumber = senderPhone;
         } else {
            // This is an SMSLISTENREQUEST
            SMSListenRequestPacket* slp = (SMSListenRequestPacket *)p;
            decodingSuccess = slp->getPhoneNumber(compareNumber) > 0;
         }
         if ( decodingSuccess ) {
            int phoneNumberLength = strlen( compareNumber );
            // Data decoded OK
            SMSModuleNotice* bestModule = NULL; // The module that is the best
            uint32 bestStatistics = MAX_UINT32;
            
            for( ModuleList::iterator it = begin();
                 it != end();
                 ++it ) {
               SMSModuleNotice* mn = static_cast<SMSModuleNotice*>(*it);
               bool found = false;
               // Now check if this module has the right telephonenumber
               // or service name
               if ( phoneNumberLength == 0 ) {
                  // Any module with few messages in the queue
                  // will do.
                  found = true;
               } else {
                  char** phones = mn->getPhoneNumbers();
                  for( int i=0; i < mn->getNumberOfPhones() && !found; i++ ) {
                     if ( strcmp(compareNumber, phones[i] ) == 0 ) 
                        found = true;
                  }
                  char ** services = mn->getServiceNames();
                  for ( int i=0; i< mn->getNumberOfServices() && !found; i++) {
                     if ( strcmp(compareNumber, services[i] ) == 0 ) 
                        found = true;
                  }
               }
               if ( found ) 
                  // A module with the right phonenumber was found.
                  if ( bestModule == NULL || 
                       ( mn->getPacketsInQueue() + mn->getRequests())
                       <= bestStatistics ) {
                     bestModule = mn;
                     bestStatistics = mn->getPacketsInQueue() +
                        mn->getRequests();
                  }
            }
            return bestModule;
         }  else {
            DEBUG1(cerr << "SMSModuleList::getBestModule - couldn't decode"
                   " SMSSendRequestPacket" << endl);
            return NULL;
         }
      } else {
         DEBUG2(cerr << "  SMSModuleList::getBestModule() - "
                "wrong packettype: " << p->getSubTypeAsString() << endl);
         return NULL;
      }
   } else {
      DEBUG2(cerr << "  SMSModuleList::getBestModule() - packet == NULL!"
             << endl);
      return NULL;
   }
}
