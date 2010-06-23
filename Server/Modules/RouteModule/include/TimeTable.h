/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TIME_TABLE_H
#define TIME_TABLE_H

#include "Vector.h"

/**
 * Class for handling a timetable
 *
 */
class TimeTable
{
	public:

      /**
       * Default constructor.
       */ 
      TimeTable();

		/** 
		 * Constructor.
		 *
		 * @param lineID is the ID of the line.
		 */
		TimeTable( uint16 lineID );

		/** 
		 * Adds an entry in the timetalbe list.
		 * 
		 * @param time is the time entry.
		 */
		void addTime( uint32 time );

      /** 
       * Returns the next time in the timetable.
       *
       * @param time is the time to lookup in the timetable.
       * @return the next time in the timetable.
       * 
       */
      uint32 getNextTime( uint32 time );

      /**
       * @param lineID is the new line ID
       */
      inline void setLineID( uint16 lineID );

      /**
       * @name Operator, to search the elements.
       */
      //@{
      /// equal
      bool operator == (const TimeTable& elm) const; 

      /// not equal
      bool operator != (const TimeTable& elm) const; 

      /// less than
      bool operator  < (const TimeTable& elm) const; 

      /// larger than
      bool operator  > (const TimeTable& elm) const; 
      //@}

   protected:
      
      /**
       * The Id of the bus line.
       */
      uint16 m_lineID;

      /**
       * A vector with the current time table.
       */
      Vector m_time;
};

inline void TimeTable::setLineID( uint16 lineID )
{
   m_lineID = lineID;
}

#endif


