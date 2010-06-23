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

#include "ParserRouteHandler.h"

#include <memory>

#include "RoutePacket.h"
#include "ParserThread.h"
#include "SubRouteVector.h"
#include "UserData.h"
#include "RouteRequest.h"
#include "DriverPref.h"
#include "RouteRequestParams.h"
#include "UnexpandedRoute.h"

#undef TRACE
#define TRACE                                                           \
do {                                                                    \
   mc2dbg << "[ParserRouteHandler:" << __LINE__ << "]" << endl;         \
} while(0)


// TODO: WARNING This file was missing all error checks... 
//       I added some but this class needs a rewrite to handle errors 


ParserRouteHandler::ParserRouteHandler( ParserThread* thread,
                                        ParserThreadGroup* group )
   : ParserHandler( thread, group )
{
}

ParserRouteHandler::~ParserRouteHandler()
{
}

RouteRequest* 
ParserRouteHandler::redoRoute(const RouteID& oldRouteID,
                              const RouteRequestParams& rrParams,
                              const RequestUserData& user,
                              const TopRegionRequest* topReq,
                              const DisturbanceList* disturbances)
{
   //find the origin and destination of the old route

   //the old route reply
   auto_ptr<RouteReplyPacket> oldRouteReply( 
                               m_thread->getStoredRoute( oldRouteID ) );

   if ( oldRouteReply.get() == NULL ) {
      return NULL;
   }
   //we need a DriverPref to decode the routereplypacket contents
   DriverPref driverPref;
   //the vector of subroutes is extracted
   auto_ptr<SubRouteVector> oldRouteVector( 
      oldRouteReply->createSubRouteVector(&driverPref) );

   //we need the first...
   SubRoute* firstSub = oldRouteVector->getSubRouteAt(0);
   //...and last subroute.
   SubRoute* lastSub  = 
      oldRouteVector->getSubRouteAt( oldRouteVector->getSize() - 1);

   //the origin parameters
   uint32 originMap  = firstSub->getThisMapID();
   uint32 originNode = firstSub->getOrigNodeID();
   uint16 originOffset = oldRouteVector->getStartOffsetUint16();
   //unfortunately not possible to use right now. 
   //bool startDirFromZero = oldRouteVector->getStartDirFromZero();

   //the destination parameters
   uint32 destMap    = lastSub->getThisMapID();
   uint32 destNode   = lastSub->getDestNodeID();
   uint16 destOffset = oldRouteVector->getEndOffsetUint16();


   //create the routerequest
   auto_ptr<RouteRequest> rr(
      rrParams.createRouteRequest( user, m_thread->getNextRequestID(),
                                   topReq, disturbances ) );

   rr->addOriginID(      originMap, originNode, originOffset );
   rr->addDestinationID( destMap,   destNode,   destOffset   );

   m_thread->putRequest( rr.get() );

   if ( rr->getStatus() != StringTable::OK ) {
      delete rr->getAnswer();
      rr.reset(NULL); //delete the object the auto_ptr holds.
   }

   return rr.release(); //release the object and return the held pointer
}

UnexpandedRoute* 
ParserRouteHandler::redoUnexpandedRoute(const RouteID& oldRouteID,
                                        const RouteRequestParams& rrParams,
                                        const RequestUserData& user,
                                        const TopRegionRequest* topReq,
                                        const DisturbanceList* disturbances)
{
   auto_ptr<RouteRequest> rr( redoRoute( oldRouteID, rrParams, user, 
                                         topReq, disturbances ) );
   UnexpandedRoute* ur = NULL;
   if ( rr.get() != NULL ) {
      const RouteReplyPacket* redoneRouteReply = rr->getRouteReplyPacket();
      const DriverPref& prefs = rrParams.getDriverPrefs();
      auto_ptr<SubRouteVector> srv( redoneRouteReply->createSubRouteVector( &prefs ) );
      ur = new UnexpandedRoute( *srv, prefs);
      delete rr->getAnswer();
   }
   return ur;
}

UnexpandedRoute* 
ParserRouteHandler::getStoredUnexpandedRoute( const RouteID& routeID,
                                              const DriverPref& prefs )
{
   auto_ptr<RouteReplyPacket> route( m_thread->getStoredRoute( routeID ) );
   if ( route.get() == NULL ) return NULL;
   auto_ptr<SubRouteVector> srv( route->createSubRouteVector(&prefs) );
   return new UnexpandedRoute(*srv, prefs);
}

bool ParserRouteHandler::routeChanged( const RouteID& oldRouteID,
                                       const RouteRequestParams& rrParams,
                                       const RequestUserData& user,
                                       const TopRegionRequest* topReq )
{
   //if the oldrouteid is not valid, we really shouldn't have called
   //this function. Best to do nothing and return true.
   bool changed = !oldRouteID.isValid();
   if ( oldRouteID.isValid() ){
      // temp variable for topregion
      const TopRegionRequest* topRegion = m_thread->getTopRegionRequest();
      if ( topRegion != NULL ) {
         //redo the old route.
         auto_ptr<UnexpandedRoute> redoneUR(
            redoUnexpandedRoute( oldRouteID, rrParams, user, topRegion ) );

         // load old unexpandedroute
         auto_ptr<UnexpandedRoute> oldUR(
            getStoredUnexpandedRoute( oldRouteID, 
                                      rrParams.getDriverPrefs() ) );
         //compare routes. 
         if ( oldUR.get() != NULL && redoneUR.get() != NULL ) {
            //compareToOriginal returns true if the routes are different. 
            changed = redoneUR->compareToOriginal(*oldUR);
         } else {
            // Nothing to compare too, I flip the coin and say changed
            // Route is old it must need an update.
            changed = true;
         }
      } else {
         // Try again later... don't know how to signal that so...
         changed = true;
      }
   }

   return changed;
}
