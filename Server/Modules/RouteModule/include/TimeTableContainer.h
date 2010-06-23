/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TIME_TABLE_CONTAINER_H
#define TIME_TABLE_CONTAINER_H

#include "TimeTable.h"

/**
 * Class for handling a timetables
 *
 */
class TimeTableContainer
{
   public:
      /**
       * Default constructor.
       */
      TimeTableContainer();

      /** 
       * Add a new timetable to the container.
       *
       * @param timeTable is the new timetable to add.
       */
      void addTimeTable( TimeTable* timeTable );

      /**
       * Return the correct timetable.
       *
       * @param lineID is the id of the line to get the timetable for.
       * @return the timetable or NULL if not found.
       */
      TimeTable* getTimeTable( uint16 lineID );

   protected:

      typedef std::vector< TimeTable* > TimeTableVector;

      /**
       * A vector with the timetables.
       */
      TimeTableVector m_timeTables;

      /**
       * A timetable item used to find an item.
       */
      TimeTable m_searchTimeTable;
};

#endif
