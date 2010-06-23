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
#include "SMSModuleNotice.h"


SMSModuleNotice::SMSModuleNotice(StatisticsPacket* sp)
      : ModuleNotice(sp)
{
   m_statisticsPacket = NULL;
   m_phoneNumbers     = NULL;
   m_serviceNames     = NULL;
   SMSModuleNotice::updateData(sp);
}


void
SMSModuleNotice::updateData(StatisticsPacket* sp)
{
   ModuleNotice::updateData(sp);
   
   delete m_statisticsPacket;
   m_statisticsPacket = (SMSStatisticsPacket*)(sp->getClone());
   m_numberOfPhones   = m_statisticsPacket->getNumberOfPhoneNumbers();
   m_numberOfServices = m_statisticsPacket->getNumberOfServiceNames();
   delete m_phoneNumbers;
   m_phoneNumbers = new char*[m_numberOfPhones];
   delete m_serviceNames;
   m_serviceNames = new char*[m_numberOfServices];
   m_statisticsPacket->getPhonesAndServices(m_phoneNumbers, m_serviceNames);
      
   m_timeForStatistics = TimeUtility::getCurrentTime();

   // Write out the data
   mc2dbg8 << "  SMSModuleNotice updated with" << endl
           << "  -- numberOfPhones = " << m_numberOfPhones << endl
           << "  -- numberOfServices = " << m_numberOfServices << endl;
   DEBUG8(if ( m_numberOfPhones > 0 )
             mc2dbg << "  -- phoneNumbers: " << endl;
          for (int i = 0 ; i < m_numberOfPhones; i++)
             mc2dbg << "  -- " << m_phoneNumbers[i] << endl;
          );
   DEBUG8(if ( m_numberOfServices > 0 )
             mc2dbg << "  -- serviceNames: " << endl;
          for (int i = 0 ; i < m_numberOfServices; i++)
             mc2dbg << "  -- " << m_serviceNames[i] << endl;
          );
}


SMSModuleNotice::~SMSModuleNotice()
{
   delete [] m_phoneNumbers;
   m_phoneNumbers = NULL;
   delete [] m_serviceNames;
   m_serviceNames = NULL;
   delete m_statisticsPacket;
   m_statisticsPacket = NULL;
}




