/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DEBITMAKING_H
#define DEBITMAKING_H

#include "config.h"

class MC2Coordinate;
class SearchRequest;
class SearchResultRequest;
class RouteRequest;
class VanillaMatch;
class RouteItem;
class ProximityRequest;
class PacketContainer;
class SearchRequestParameters;


/**
 * Utility class for making debit strings.
 *
 */
class DebitMaking {
   public:
      /**
       * Make debiting information for one search.
       *
       * @param operation The string to print the debiting into, must 
       *                  be 256 long. 
       * @param inReq The SearchRequest to debit for.
       * @param locationString The location searched for in SearchRequest.
       * @param searchString The searchString in the SearchRequest.
       * @return True if debit string filled ok, false if not.
       */
      static bool makeSearchDebit( char* operation,
                                   SearchResultRequest* inReq,
                                   const char* const locationString, 
                                   const char* const searchString,
                                   const char* extraString = NULL );


      /**
       * Make debiting information for one proximity serach.
       *
       * @param operation The string to print the debiting into, must 
       *                  be 256 long. 
       * @param inReq The ProximityRequest to debit for.
       * @param ansCont The answer PacketContainer with a 
       *                VanillaSearchReply or a ProximityReply.
       * @param searchString The searchString in the ProximityRequest. NULL
       *                     if searchString was not used in search.
       * @param lat The latitude of the centerpoint in the circle 
       *            proximity area.
       * @param lon The longitude of the centerpoint in the circle 
       *            proximity area.
       * @param distance The distance from the centerpoint searched.
       * @return True if debit string filled ok, false if not.
       */
      static bool makeProximityDebit( char* operation,
                                      ProximityRequest* inReq,
                                      PacketContainer* ansCont,
                                      const char* const searchString,
                                      int32 lat, int32 lon, 
                                      uint32 distance );


      /**
       * Make debiting information for one proximity serach.
       *
       * @param operation    The string to print the debiting into, must 
       *                     be 256 long. 
       * @param searchReq    The SearchResultRequest to debit for.
       * @param params       The search params used.
       * @param searchString The searchString in the ProximityRequest. NULL
       *                     if searchString was not used in search.
       * @param coord        The center coordinate.
       * @param distance     The distance from the centerpoint searched.
       * @return True if debit string filled ok, false if not.
       */
      static bool makeProximityDebit( char* operation,
                                      const SearchResultRequest* searchReq,
                                      const SearchRequestParameters& params,
                                      const char* const searchString,
                                      const MC2Coordinate& coord,
                                      uint32 distance );


      /**
       * Make debiting information for one route. Using VanillaMatch to 
       * identify origin and destination.
       *
       * @param operation The string to print the debiting into, must 
       *                  be 256 long. 
       * @param inReq The RouteRequest to debit for.
       * @param originVM The origin of the route.
       * @param destinationVM The destination of the route.
       * @param rerouteID The routeID that this route is a re-route for.
       * @param rerouteCreateTime The route create time that this route
       *                          is a re-route for.
       * @return True if debit string filled ok, false if not.
       */
      static bool makeRouteDebit( 
         char* operation,
         RouteRequest* inReq, 
         const VanillaMatch* const originVM, 
         const VanillaMatch* const destinationVM,
         uint32 rerouteID = 0, 
         uint32 rerouteCreateTime = 0,
         uint32 rerouteReason = 0 );


      /**
       * Make debiting information for one route. Using Using RouteItem to 
       * identify origin and destination.
       *
       * @param operation The string to print the debiting into, must 
       *                  be 256 long.
       * @param inReq The RouteRequest to debit for.
       * @param nbrRouteItems The number of RouteItems.
       * @param theRouteItems The RouteItems.
       * @param rerouteID The routeID that this route is a re-route for.
       * @param rerouteCreateTime The route create time that this route
       *                          is a re-route for.
       * @return True if debit string filled ok, false if not.
       */
      static bool makeRouteDebit( 
         char* operation,
         RouteRequest* inReq,
         uint32 nbrRouteItems, RouteItem** theRouteItems,
         uint32 rerouteID = 0, 
         uint32 rerouteCreateTime = 0,
         uint32 rerouteReason = 0 );


      /**
       * Make debiting information for one route. Using Using RouteRequests
       * valid-orig/-dest to identify origin and destination.
       *
       * @param operation The string to print the debiting into, must 
       *                  be 256 long.
       * @param inReq The RouteRequest to debit for.
       * @param rerouteID The routeID that this route is a re-route for.
       * @param rerouteCreateTime The route create time that this route
       *                          is a re-route for.
       * @return True if debit string filled ok, false if not.
       */
      static bool makeRouteDebit( char* operation, RouteRequest* inReq,
                                  uint32 rerouteID = 0, 
                                  uint32 rerouteCreateTime = 0,
                                  uint32 rerouteReason = 0 );


   private:
      /**
       * Private constructor to avoid use.
       */
      DebitMaking();
};

#endif // DEBITMAKING_H

