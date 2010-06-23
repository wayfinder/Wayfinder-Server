/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TimeTable.h"

TimeTable::TimeTable()
{
   m_lineID = MAX_UINT16;
}

TimeTable::TimeTable( uint16 lineID )
   : m_time( 16, 16 )
{
	m_lineID = lineID;
}

void TimeTable::addTime( uint32 time )
{
	m_time.addLast( time );
}

uint32 TimeTable::getNextTime( uint32 time )
{
   uint32 left = 0;
   uint32 right = m_time.getSize();
   uint32 mid = (left + right) / 2;

   while (left < right) {

      if (m_time[mid] < time) 
         right = mid - 1;
      else if (m_time[mid] > time) 
         left = mid+1;
      else
         return m_time[mid];
         
      mid = (left + right) / 2;
   }
   if( mid < m_time.getSize() )
      return m_time[mid];
   else
      return MAX_UINT16;
}

bool TimeTable::operator == (const TimeTable& elm) const 
{
   return ( m_lineID == elm.m_lineID );
}

bool TimeTable::operator != (const TimeTable& elm) const 
{
   return ( m_lineID != elm.m_lineID );
}

bool TimeTable::operator > (const TimeTable& elm) const 
{
   return ( m_lineID > elm.m_lineID );
}

bool TimeTable::operator < (const TimeTable& elm) const 
{
   return ( m_lineID < elm.m_lineID );
}
