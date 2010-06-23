/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "SQLTrafficElementStrings.h"

#include "DisturbanceElement.h"

#include "StringUtility.h"
#include "SQLStringStream.h"
#include "MapBits.h"

#include <boost/lexical_cast.hpp>

namespace SQLTrafficElementStrings {

MC2String createDeleteQuery( const MC2String& table,
                             const TrafficElementDatabase::
                             TrafficElements& removeElements ) {
   SQL::StringStream query;
   query << "DELETE FROM " << table.c_str() << " WHERE situationReference IN(";
   for ( TrafficElementDatabase::TrafficElements::const_iterator
            it = removeElements.begin();
         it != removeElements.end(); ++it ) {
      if ( it != removeElements.begin() ) {
         query << ", ";
      }
      query << (*it)->getSituationReference();
   }
   // add empty string for empty
   if ( removeElements.empty() ) {
      query << "''";
   }

   query << ")";

   return query.str();
}

void createDisturbanceValues( SQL::StringStream& stream,
                              const DisturbanceElement& element ) {
   stream << "("
          << element.getDisturbanceID() << ", "
          << element.getSituationReference() << ", "
          << element.getType() << ", "
          << element.getPhrase() << ", "
          << element.getEventCode() << ", "
          << element.getStartTime() << ", "
          << element.getEndTime() << ", "
          << element.getCreationTime() << ", "
          << element.getSeverity() << ", "
          << element.getDirection() << ", "
          << StringUtility::SQLEscapeSecure( element.getText() ) << ", "
          << element.getDeleted() << ", "
          << element.getFirstLocation() << ", "
          << element.getSecondLocation() << ", "
          << element.getExtent() << ", "
          << element.getCostFactor() << ", "
          << element.getQueueLength() << ")";
}

TrafficElementDatabase::TrafficElements::const_iterator 
createAddQueryDisturbance( 
      TrafficElementDatabase::TrafficElements::const_iterator begin,
      const TrafficElementDatabase::TrafficElements::const_iterator end,
      MC2String& queryString,
      const size_t queryMaxSize ) {
   SQL::StringStream query;
   query << "REPLACE INTO ISABDisturbance"
         << "(disturbanceID, situationReference, type, phrase, eventCode,"
         << " startTime, endTime, creationTime,"
         << " severity, direction, text, deleted, firstLocation,"
         << " secondLocation, extent, costFactor, queueLength)"
         <<  " VALUES";

   // For all disturbances; create table values
   bool processedElement = false;
   for ( ; begin != end; ++begin ) {
      // only write data that is from the underview maps, since the information
      // on the overview maps are based on this information.
      if ( MapBits::isUnderviewMap( (*begin)->getMapID() ) ) {
         // add "," after all new elements, except before the first and the
         // last element.
         if ( processedElement ) {
            query << ",";
         }

         // create disturbance values for current disturbance
         createDisturbanceValues( query, **begin );
         processedElement = true;
      }
      //  check the query size limit, this is done so we can be sure that the
      //  query size wont be bigger then the size set in 'max_allowed_packet'
      //  size in the data base.
      if ( query.str().size() > queryMaxSize ) {
         queryString = query.str();
         return ++begin;
      }
   }

   // needed since there is a possibillity that all the elements are not
   // from underview maps which results in a "empty" query.
   if ( processedElement ) {
      queryString = query.str();
   }
   
   return begin;
}

void createCoordValues( SQL::StringStream& stream,
                        const DisturbanceElement& element ) {
   typedef DisturbanceElement::RouteIndex2CoordMap RouteIndex2CoordMap;
   typedef DisturbanceElement::RouteIndex2AngleMap RouteIndex2AngleMap;
   const RouteIndex2CoordMap& latVector = element.getLatMap();
   const RouteIndex2CoordMap& lonVector = element.getLonMap();
   const RouteIndex2AngleMap& angleVector = element.getAngle();
   const DisturbanceElement::RouteIndex& indexVector = element.getRouteIndex();
   // For all route indexes; create coord table values
   for ( DisturbanceElement::RouteIndex::const_iterator
            indexIt = indexVector.begin(); 
         indexIt != indexVector.end();
         ++indexIt) {
      // add "," after all new elements, except the last
      if ( indexIt != indexVector.begin() ) {
         stream << ",";
      }
      RouteIndex2CoordMap::value_type::first_type routeIndex = *indexIt;
      RouteIndex2CoordMap::const_iterator latlonIt =
         latVector.find( routeIndex);

      RouteIndex2CoordMap::value_type::second_type latitude = latlonIt->second;
      latlonIt = lonVector.find( routeIndex );
      RouteIndex2CoordMap::value_type::second_type longitude = latlonIt->second;
      RouteIndex2AngleMap::const_iterator it = angleVector.find( routeIndex );
      RouteIndex2AngleMap::value_type::second_type angle = it->second;

      stream << "(" << element.getDisturbanceID()
             << ", " << element.getSituationReference() << ", "
             << latitude << ", " << longitude << ", " << angle << ", "
             << routeIndex << ")";

   }

}

TrafficElementDatabase::TrafficElements::const_iterator 
createAddQueryCoords( 
      TrafficElementDatabase::TrafficElements::const_iterator begin,
      const TrafficElementDatabase::TrafficElements::const_iterator end,
      MC2String& queryString,
      const size_t queryMaxSize ) {

   SQL::StringStream query;
   query << "REPLACE INTO ISABDisturbanceCoords("
         << "disturbanceID, situationReference, "
         << "latitude, longitude, angle, routeIndex) VALUES";

   // For all disturbances; create coord values for table
   bool processedElement = false;
   for ( ; begin != end; ++begin ) {
      // only write data that is from the underview maps, since the information
      // on the overview maps are based on this information.
      if ( MapBits::isUnderviewMap( (*begin)->getMapID() ) ) {
         // add "," after all new elements, except before the first and the
         // last element.
         if ( processedElement ) {
            query << ",";
         }

         // create coord values for current disturbance
         createCoordValues( query, **begin );
         processedElement = true;
      }

      //  check the query size limit, this is done so we can be sure that the
      //  query size wont be bigger then the size set in 'max_allowed_packet'
      //  size in the data base.
      if ( query.str().size() > queryMaxSize ) {
         queryString = query.str();
         return ++begin;
      }
   }

   // needed since there is a possibillity that all the elements are not
   // from underview maps which results in a "empty" query.
   if ( processedElement ) {
      queryString = query.str();
   }

   return begin;
}

} // SQLTrafficElementStrings
