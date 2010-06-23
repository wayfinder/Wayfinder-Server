/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef MAIN_METHOD_RANDTEST

#include "config.h"
#include "RouteReader.h"
#include "RouteProcessor.h"
#include "TCPSocket.h"
#include "JobThread.h"
#include "multicast.h"
#include "SubRoutePacket.h"
#include "CoordinateOnItemPacket.h"

#include <time.h>
#include <iostream>
#include <stdio.h>

/**
 * Programs that runs the nodes in the file randomnodes. The map has to be the
 * same. For each route, the number of connections in the optimal path between
 * origin and destination, the number of visited nodes and the time to compute
 * the route is stored. The first column in routeIDs contains the number of
 * nodes in a route.
 *
 * The output from a program is in randtestresult. The routes are in routeIDs.
 *
 * Usage: ./RandomNodesTest mapID [dividingFactor]
 * where mapID is the ID of the map and dividingFactor is the (optionally
 * given) number of routes between each "." on the screen.
 */
int main(int argc, char* argv[])
{
   if ((argc < 2) || (argc > 3)) {
      mc2log << info
             << "Usage: ./RandomNodesTest mapID [dividingFactor]" << endl;
      exit(1);
   }

   uint16 mapID = atoi(argv[1]);
   int dividingFactor;
   if (argc == 3)
      dividingFactor = atoi(argv[2]);
   else
      dividingFactor = 100;

   clock_t totalTime = 0;

   SafeVector* loadedMaps = new SafeVector();
   RouteProcessor* routeProcessor = new RouteProcessor(loadedMaps);

   Packet *replyPacket, *lMReqPacket;

      // Load the map
   lMReqPacket = new LoadMapRequestPacket(
      mapID,
      ntohl( inet_addr( 
         MultiCastProperties::getIP( MultiCastProperties::MAP_LEADER ) ) ),
      MultiCastProperties::getPort( MultiCastProperties::MAP_LEADER ) );
   replyPacket = routeProcessor->handleRequest((RequestPacket*)lMReqPacket);
   delete replyPacket;

      // Create a connection to the Map Leader
   DatagramSender *socket = new DatagramSender;
   DatagramReceiver *receiver =
      new DatagramReceiver(8000, DatagramReceiver::FINDFREEPORT);

   uint32 mapIP = 
      ntohl( inet_addr(
         MultiCastProperties::getIP( MultiCastProperties::MAP_LEADER ) ) );
   uint16 mapPort = 
      MultiCastProperties::getPort( MultiCastProperties::MAP_LEADER );

      // Create som other misc stuff
   SubRouteRequestPacket* packet;
   
   FILE* routeFile;
   FILE* resultFile;
   FILE* routeIDsFile;

   routeFile    = fopen("randomnodes", "r");
   resultFile   = fopen("randtestresult", "w");
   routeIDsFile = fopen("routeIDs", "w");

   int i = 0;
   while (!feof(routeFile)) {
      if ((i++ % dividingFactor) == 0)
         mc2log<< info << "." << flush;
      
      mc2dbg1 << i << " " << flush;
      
      uint32 originItemID = 0;
      uint32 destinationItemID = 0;
      fscanf(routeFile, "%u %u\n", &originItemID, &destinationItemID);

      mc2dbg1 << "Origin and destination item IDs:" << endl
              << originItemID << " " << destinationItemID << endl;

         // Ask the Map Leader for the latitudes and longitudes
      int32 origLat  = 0;
      int32 origLong = 0;
      int32 destLat  = 0;
      int32 destLong = 0;

      CoordinateOnItemRequestPacket* itemRequest = 
         new CoordinateOnItemRequestPacket();
      itemRequest->setOriginIP(NetUtility::getLocalIP());
      itemRequest->setOriginPort(receiver->getPort());
      itemRequest->setMapID(mapID);
      itemRequest->add(REMOVE_UINT32_MSB(originItemID), 0);
      itemRequest->add(REMOVE_UINT32_MSB(destinationItemID), 0);

      if (!socket->send(itemRequest, mapIP, mapPort))
         PANIC("RandomNodesTest:",
               "Error couldn't reach Map Leader");

      delete itemRequest;
      CoordinateOnItemReplyPacket* itemReply = 
         new CoordinateOnItemReplyPacket();

      if (!receiver->receive(itemReply, 20000000))
         PANIC("ReadRoutingResult:",
               "Error receiving ack (gets no itemReply)");

      uint32 itemReplyID;
      itemReply->getLatLong(0, itemReplyID, origLat, origLong);
      itemReply->getLatLong(1, itemReplyID, destLat, destLong);

      delete itemReply;
      
         // Create a packet
      RouteRequestPacket* routePack = new RouteRequestPacket;
                                        
      routePack->setCustomerParam(0, 0, 1, 0, 0, 0);
      routePack->setVehicleParam(1);

         // Add origin
      routePack->add(true, true, 0, 0, mapID, originItemID);
      routePack->setLatLong(0, origLat, origLong);

         // Add destination
      routePack->add(false, true, 0, 0, mapID, destinationItemID);
      routePack->setLatLong(1, destLat, destLong);


      packet = new SubRouteRequestPacket( routePack,
                                          4,
                                          false,
                                          1 );

         // Calculate the route
      clock_t startTime = clock();
      replyPacket = routeProcessor->handleRequest(packet);
      clock_t endTime = clock();

         // Collect some data
      clock_t timeDiff = endTime - startTime;
      totalTime += timeDiff;

/*
      CalcRoute* calc = routeProcessor->getCalcRoute( 0 );
      RoutingMap* routingMap = calc->getMap();
      RoutingNode *origin, *destination;
      
      uint32 nbrItems = ((SubRouteReplyPacket*)replyPacket)->;
//getNbrItems();

      origin      = routingMap->getNodeFromTrueNodeNumber(originItemID);
      destination = routingMap->getNodeFromTrueNodeNumber(destinationItemID);

      fprintf(resultFile, "%u          %u            %ld\n",
              nbrItems,
              calc->getNodesVisited(),
              timeDiff);

      fprintf(routeIDsFile, "%u ", nbrItems);
      uint32 mapid;     // This is a dummy only
      uint32 itemID;
      for (uint32 i = 0; i < nbrItems; i++) {
         ((SubRouteReplyPacket*)replyPacket)->getRouteItem(i, mapid, itemID);
         fprintf(routeIDsFile, "%u ", itemID);
      }
      fprintf(routeIDsFile, "\n");
  
*/    
      delete replyPacket;
   } // end_while

   fclose(routeFile);
   fclose(resultFile);
   fclose(routeIDsFile);

   delete routeProcessor;
   delete loadedMaps;

   mc2log << info << "\nTotal time in RouteProcessor::handleRequest(): "
        << (1000 * totalTime / CLOCKS_PER_SEC) << " ms." << endl;
} // end_main

#endif
