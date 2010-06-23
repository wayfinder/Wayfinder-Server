/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTEREQUESTPARAMS_H
#define ROUTEREQUESTPARAMS_H

#include "config.h"
#include "StringTable.h"
#include "DriverPref.h"

//forward declarations of used classes.
class RequestUserData;
class TopRegionRequest;
class DisturbanceList;
class RouteAllowedMap;
class RouteRequest;
class Request;

/**
 * This class holds parameters that RouteRequest needs in order to
 * create new routes. Objects of this class may be constructed freely
 * or extracted from an existing RouteRequest. Once a
 * RouteRequestParams object is set up, it can be used to creat new
 * RouteRequest objects.
 *
 * This class is copy-constructable and assignable using the default
 * copy constructor and assignment operator.
 */
class RouteRequestParams {
public:
   /** @name Constructors. */
   //@{
   /**
    * Default constructor.  Equal to
    * <code>
    * RouteRequestParams(0, StringTable::ENGLISH, 1, false, 0)
    * </code>
    */
   RouteRequestParams();

   /**
    * Constructor.
    */
   RouteRequestParams( uint32 expandType,
                       StringTable::languageCode language,
                       bool noConcatenate,
                       uint32 passedRoads,
                       uint32 nbrWantedRoutes );
   //@}

   class RouteRequestParams& setAllowedMaps( RouteAllowedMap* maps )
   {
      m_allowedMaps = maps;
      return *this;
   }

   class RouteRequestParams& 
   setCompareDisturbanceRoute( bool compareDisturbance  )
   {
      m_compareDisturbanceRoute = compareDisturbance;
      return *this;
   }

   class RouteRequestParams& setNameChangeAsWP( bool nameChangeAsWP )
   {
      m_nameChangeAsWP = nameChangeAsWP;
      return *this;
   }

   class RouteRequestParams& setRemoveAheadIfDiff(bool removeAheadIfDiff )
   {
      m_removeAheadIfDiff = removeAheadIfDiff;
      return *this;
   }

   const DriverPref& getDriverPrefs() const
   {
      return m_driverPrefs;
   }

   /**
    * Sets all the parameters *except* the ones set by the constructor. 
    * @param rr The RouteRequest that will be altered. 
    */
   //   bool setParamsToRequest(RouteRequest& rr) const;
   /**
    * Creates a new RouteRequest using the parameters set in this
    * object.
    */
   RouteRequest* createRouteRequest(const RequestUserData& user,
                                    uint16 reqID, 
                                    const TopRegionRequest* topReq,
                                    const DisturbanceList* disturbances = NULL,
                                    Request* parentRequest = NULL) const;
protected:


   /** @name Parameters refactored from RouteRequestData. */
   //@{
   /**
    *   True if the expanded route should be abbreviated.
    */
   bool m_abbreviate;

   /**
    *   True if landmarks should be used.
    */
   bool m_landmarks;

   /**
    *   Unknown use.
    */
   bool m_isStartTime;

   /**
    *   Cost for starting in the opposite direction.
    */
   uint32 m_turnCost;

   /**
    *   Time. Unknown which.
    */
   uint32 m_time;

   /**
    *   Type of expansion wanted.
    */
   uint32 m_expandType;

   /**
    *   Language.
    */
   StringTable::languageCode m_language;

   /**
    *   The number of wanted routes.
    */
   uint32 m_nbrWantedRoutes;

   /**
    *   Pointer to the map of allowed maps.
    */
   RouteAllowedMap* m_allowedMaps;

   /**
    *   True if some ahead turns should not be removed.
    */
   bool m_noConcatenate;

   /**
    *   I don't know. There was no explanation for
    *   the parameter.
    */
   uint32 m_passedRoads;

   /**
    *   Set to true if toll roads should be avoided.
    */
   bool m_avoidTollRoads;

   /**
    *   Set to true if highway roads should be avoided.
    */
   bool m_avoidHighways;

   /**
    *   If true, aheads should be removed even if the name
    *   changes.
    */
   bool m_removeAheadIfDiff;

   /**
    *   Set if name changes should be included as waypoints.
    */
   bool m_nameChangeAsWP;

   /**
    * Set if to route with and without disturbances.
    */
   bool m_compareDisturbanceRoute;

   /**
    *   The driver preferences.
    */
   DriverPref m_driverPrefs;

   //@}
};
#endif

