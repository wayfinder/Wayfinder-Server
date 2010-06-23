/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EVENTCODELIST_H
#define EVENTCODELIST_H

#include "config.h"
#include "TrafficDataTypes.h"
#include "MC2String.h"
#include <algorithm>

class EventCodeList {

  public:
   /**
    *   One entry in a event code table
    */
   struct eventCode_entry {
      ///  The event code
      const uint32 eventCode;
      ///  The mapped disturbance type
      const TrafficDataTypes::disturbanceType disturbanceType;
      ///  A descrption of the event
      const char* const description;
      ///  The mapped severity
      const TrafficDataTypes::severity severity;
   };

   /**
    *   A table of eventCode entry. Also contains the number of entrties
    *   in the table.
    */
   struct eventCodeTable {
      uint32 nbrEntries;
      const eventCode_entry* const entries;
   };
   
   class EventCodeEntryComp {
     public:
      bool operator()(const eventCode_entry& a, const eventCode_entry& b) {
         return a.eventCode < b.eventCode;
      }
      bool operator()(const eventCode_entry& a, uint32 code) {
         return a.eventCode < code;
      }
   };
   
   /// Result of a findInTable operation.
   struct eventCodeTableFindRes {
      eventCodeTableFindRes( TrafficDataTypes::disturbanceType theType,
                             TrafficDataTypes::severity theSeverity )
            : type(theType), severity(theSeverity) {}
      TrafficDataTypes::disturbanceType type;
      TrafficDataTypes::severity severity;
   };
   
   /**
    *   Looks for the code <code>code</code> in the table <code>table</code>.
    *   @param table Table to search.
    *   @param code  Code to look for.
    *   @return Result with string set to NULL and length set to zero
    *           if not found.
    */
   static eventCodeTableFindRes findInTable( const eventCodeTable& table,
                                             uint32 code ) {
      const eventCode_entry* begin = table.entries;
      const eventCode_entry* end   = table.entries + table.nbrEntries;
      const eventCode_entry* found = std::lower_bound( begin, end,
                                                        code,
                                                        EventCodeEntryComp() );
      if ( found->eventCode == code ) {
         return eventCodeTableFindRes(found->disturbanceType,
                                      found->severity );
      } else {
         return eventCodeTableFindRes(TrafficDataTypes::NoType,
                                      TrafficDataTypes::NoSeverity);
      }
   }

   // The table with the event codes entries.
   static eventCodeTable c_eventCodeTable;
   
private:
   // An array with all the event code entries.
   static eventCode_entry c_eventCodeEntries [1373];  

};
#endif
