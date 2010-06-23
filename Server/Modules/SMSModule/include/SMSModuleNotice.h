/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SMSMODULENOTICE4711_H
#define SMSMODULENOTICE4711_H

#include "config.h"
#include "ModuleList.h"

class SMSStatisticsPacket;

/**
  *   Describes statistics information about a module.
  *   Contains the phoneNumbers and serviceNames for a module.
  *
  */
class SMSModuleNotice : public ModuleNotice
{
   private:
      /**
       *    Create a new module notice object. Private to avoid usage.
       */
      SMSModuleNotice();

   public:
      /**
        *   Creates a new ModuleNotice and initiates the values to
        *   those in the packet send as parameter.
        *   Doesn't call the ModuleNotice constructor.
        */
      SMSModuleNotice(StatisticsPacket* sp);

      /**
       *    Deletes this SMSModuleNotice.
       */
      virtual ~SMSModuleNotice();
      
      /**
        *   Updates the statistics (the stat-value and the loaded maps).
        */
      void updateData(StatisticsPacket* sp);

      /**
        *   Method for debugging.
        */
      void printServicesAndPhones() {
      }

      /**
       *    @return The number of phonenumbers.
       */
      int getNumberOfPhones() const {
         return m_numberOfPhones;
      };

      /**
       *    @return The number of services.
       */
      int getNumberOfServices() const {
         return m_numberOfServices;
      }

      /**
       *    @return The array with the phonenumbers.
       */
      char** getPhoneNumbers() const {
         return m_phoneNumbers;
      };


      /**
       *    @return The array containing the service names.
       */
      char **getServiceNames() const {
         return m_serviceNames;
      };
      
   private:

      /**   
       *    A pointer to the SMSStatisticsPacket. Stores the actual
       *    phonenumbers and servicenames.
       */
      SMSStatisticsPacket* m_statisticsPacket;
      
      /**
       *    Array containing pointers to the phonenumbers stored in the
       *    SMSStatisticsPacket.
       */
      char** m_phoneNumbers;

      /**
       *    Same as phoneNumbers but for servicenames.
       */ 
      char** m_serviceNames;

      /**
       *    The size of phoneNumbers.
       */
      int m_numberOfPhones;


      /**
       *    The size of serviceNames.
       */
      int m_numberOfServices;
      
};

#endif // SMSMODULELIST_H

