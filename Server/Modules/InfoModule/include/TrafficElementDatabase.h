/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFIC_ELEMENT_DATABASE_H
#define TRAFFIC_ELEMENT_DATABASE_H

#include "config.h"
#include "MC2String.h"

#include <vector>

class DisturbanceElement;
class DisturbanceChangeset;
/**
 * Interface to communicate traffic elements to a database.
 *
 */
class TrafficElementDatabase {
public:

   /// Bitmask for updateChangeset
   enum UpdateStatus {
      OK = 0,             ///< The entire changeset was successful.
      UPDATE_FAILED = 1,  ///< Update or add elements failed.
      REMOVE_FAILED = 2   ///< Remove elements failed.
   };

   /// Array type of Disturbance elements used for update changeset.
   typedef std::vector<DisturbanceElement*> TrafficElements;
   /**
    * Fetch all disturbances for a specific \c provider.
    * @param providerID A string that identifies the \c provider.
    * @param dist Will contain the \c providers disturbances.
    * @return true if fetch was successful.
    */
   virtual bool fetchAllDisturbances( const MC2String& providerID,
                                      TrafficElements& dist ) = 0;
   
   /**
    * Update changes in database by adding new and/or removing old.
    * @param newElements A set of new traffic elements to be added in database.
    * @param removeElement A set of old traffic elements to be removed in
    * database.
    * @return Status mask of the update or removed elements.
    */
   virtual int
   updateChangeset( const DisturbanceChangeset& newElements ) = 0;
};

#endif // TRAFFIC_ELEMENT_DATABASE_H
