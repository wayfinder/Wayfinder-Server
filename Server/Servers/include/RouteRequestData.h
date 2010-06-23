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

#include <vector>
#include <map>

#include "OrigDestInfo.h"
#include "StringTable.h"
#include "RouteRequestParams.h"

class RouteObject;
class ExpandedRoute;
class DisturbanceList;
class RouteAllowedMap;
class SearchMatch;
class RequestUserData;
/**
 *   Type of origins and destinations.
 *
 */
enum RROrigDestType {
   coord          = 0,
   item           = 1,
   coordAndItem   = 2
};

/**
 *   Vector to hold the origins and destinations until added
 *   to the RouteObject(s).
 */
class RROrigDestVector : public vector<pair<OrigDestInfo, RROrigDestType> > {
public:
   int addItemAndCoord( uint32 mapID, uint32 itemID, uint16 offset,
                        int32 lat, int32 lon, uint16 angle,
                        enum RROrigDestType type = coordAndItem ) {
      push_back(make_pair(OrigDestInfo(NULL, mapID, itemID, MAX_UINT32,
                             0, 0, lat, lon, angle ), type));
      back().first.setOffset( float(offset) / float( MAX_UINT16 ) );
      return size();
   }
   
   int addCoord(int32 lat, int32 lon, uint16 angle) {
      return addItemAndCoord(MAX_UINT32, MAX_UINT32, 0,
                             lat, lon, angle,
                             coord);
   }

   int addItemID(uint32 mapID, uint32 itemID, uint16 offset) {
      return addItemAndCoord(mapID, itemID, offset,
                             MAX_INT32, MAX_INT32, 0,
                             item);
   }

   
};

/**
 *    Class that holds the internal data used in the RouteRequest.
 *    Should not contain any code, just variables.
 *    The class is derived from RouteRequestParams so that it is easy
 *    to construct from a RouteRequestParams object, and they share a
 *    whole bunch of variables anyway. Not the best reason.
 */
class RouteRequestData : protected RouteRequestParams {
public:
   /**
    * Constructor. Constructs a new RouteRequestData object from a
    * RouteRequestParams object.
    * @param o The RouteRequestParams object.
    */
   RouteRequestData(const RouteRequestParams& o); 
   /**
    * Destructor.
    * Deletes all pointed-to objects, including those in the array.
    */
   ~RouteRequestData();
private:
   /**
    * Initialization function.
    */
   void init();

   enum state_t {
      INIT           =  10,
      USING_RO       =  20,
      USING_TWO_RO   =  30,
      DIST_INFO      =  35,
      EXPANDING      =  40,
      DONE           = 200,
      ERROR          = 404,
   } m_state;
   
   /// RouteRequest wants to be able to use everything.
   
   friend class RouteRequest;
   
   /**
    *    The number of route objects currently used.
    */
   int m_nbrRouteObjectsUsed;

   /**
    *    The number of route objects that we're waiting for.
    */ 
   int m_nbrRouteObjectsNotDone;
   
   /**
    *    The maximum number of route objects.
    */
   static const int m_maxNbrRouteObjects = 2;
   
   /**
    *    The RouteObject that does all the processing.
    *    The second route object is used for detecting
    *    disturbances on the routes.
    */
   RouteObject* m_routeObject[m_maxNbrRouteObjects];

    
   /**
    * The ExpandedRoute object.
    */
   ExpandedRoute* m_route;

   /**
    *   Vector of origins. Origin in first and info about
    *   what to use in second.
    */
   RROrigDestVector m_origins;

   /**
    *   Vector of destinations. Destination in first and info
    *   about what to use in second.
    */
   RROrigDestVector m_dests;

   /**
    *   List of disturbances.
    */
   const DisturbanceList* m_disturbances;

   /**
    *   Status of the request.
    */
   StringTable::stringCode m_status;

   /**
    *   Map that keeps track of which RouteObject
    *   that should get a certain replypacket.
    *   packetID in first and RouteObject in second.
    */
   map<uint32, RouteObject*> m_whoShouldGetPackets;

   /// Original origin
   SearchMatch* m_originalOrigin;
   /// Original dest
   SearchMatch* m_originalDest;
};

// -- Implementation of inlined methods.

