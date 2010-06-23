/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTELIST_4711_H
#define ROUTELIST_4711_H

#include "config.h"

#include "NavStringTable.h"

// FW-decl
class RouteReq;
class RouteElement;
class PacketContainer;
class RouteRequest;


/**
 *   Class representing a route that hopefully will be simpler to
 *   convert to a Navigator or eBox route than the expandroute result.
 *   Check the route for validity using <code>bool isValid()</code>.
 *   @see RouteElement.
 *
 */
class RouteList {
public:

   /**
    *   Creates a new RouteList.
    *   @param routeReq The routereq that was sent by the navigator.
    *   @param pc       The reply from the server.
    */
   RouteList(RouteReq* routeReq,
             PacketContainer* pc,
             RouteRequest* rr );

   /**
    *   Cleans up.
    */
   virtual ~RouteList();

   /**
    *   Populates the list using the routeReq and pc.
    *
    *   @return true if a route could be created.
    */
   bool fillList();

   /**
    *   Return the number of items in the list.
    */
   int getSize() const;

   /**
    *   Returns the total dist in the route.
    */
   int getTotalDist() const;


   /**
    * Set the total distance of the route, modules total dist is not the
    * sum of all dist in the route so that mat be needed. 
    * Think before calling this method.
    */
   void setTotalDist( uint32 dist );


   /**
    *   Returns the total time of the route.
    */
   uint32 getTotalTime( bool includeStandstill ) const;


   /**
    * Set the total time of the route, modules total time is not the
    * sum of all times in the route so that mat be needed. 
    * Think before calling this method.
    */
   void setTotalTime( uint32 newTime );


   /**
    *   Returns a ref to the  NavstringTable for the List.
    */
   NavStringTable& getStringTable();
            
   /**
    *   Returns the routeelement at position idx.
    *   @param idx The index in the vector.
    *   @return The routeElement at that position.
    */
   const RouteElement* routeElementAt(int idx) const;

   /**
    *    Get the element at position pos. {\it {\bf NB!} No check is done 
    *    against writing outside the buffer!}
    *    @param   pos   The position of the element to return.
    */
   inline const RouteElement* operator[](uint32 pos) const {
      return routeElementAt(pos);
   }
   
   /**
    *   Returns true if the route is valid.
    */
   bool isValid() const {
      return m_valid;
   }

   /**
    * Returns a pointer to the RouteRequest.
    */
   RouteReq* getReq() const {
      return m_routeReq;
   }


   /**
    *   Returns true if the route is truncated.
    */
   bool isTruncated() const;


   /**
    *   Returns the length of the route, if the route is truncated
    *   the distance is for the truncated route and not the
    *   entire roure
    */
   int getTruncatedDist() const;


   /**
    * Get the index of where the route was truncated.
    */
   uint32 getTruncatedWPTNbr() const;


   /**
    * Set if truncated.
    */
   void setTruncated( bool truncated );


   /**
    * Set length for the truncated route.
    */
   void setTruncatedDist( uint32 dist );


   /**
    * Set the index of where the route was truncated.
    */
   void setTruncatedWPTNbr( uint32 nbr );


   /**
    * Get the route request.
    */
   RouteRequest* getRouteRequest();


   protected:
      /// If the route is truncated this one is true
      bool m_truncated;


      /// The distance where the route is truncated.
      int m_truncatedDist;


      /// The WPT where the route is truncated.
      uint32 m_truncatedWPTNbr;

   
private:

   /** The vector that contains the RouteElements*/
   std::vector< RouteElement* >  m_routeVector;

   /** The RouteReq */
   RouteReq* m_routeReq;

   /** The PacketContainer */
   PacketContainer* m_pc;

   /** The stringTable */
   NavStringTable m_stringTable;

   /** True if the list contains a route */
   bool m_valid;

   /** Total dist of the route */
   uint32 m_totalDist;

   
   /**
    * Total time of the route.
    */
   uint32 m_totalTime;

   /**
    * Total standstill time of the route.
    */
   uint32 m_totalStandStillTime;

   /**
    * The route request.
    */
   RouteRequest* m_rr;
};

#endif
