/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef MAIN_METHOD_NODERAND

#include "config.h"
#include <iostream.h>
#include <cstdlib>
#include "RouteProcessor.h"
#include "RouteReader.h"
#include "RoutingNode.h"
#include "RoutingMap.h"

using namespace std;

/**
 * A method that generates origin and destination points in a map. The program
 * randomizes both points. The output from the program is a file called
 * randomnodes, where every line contains one origin and one destination.
 * In no case, origin and destination will be the same.
 *
 * Usage: ./NodeRandomizer nbrRoutes mapID [randSeed]
 * where nbrRoutes is the number of routes to generate and mapID is the map
 * that should be used.
 */
int main(int argc, char* argv[])
{
   if ((argc < 3) || (argc > 4)) {
      cerr << "Usage: ./NodeRandomizer nbrRoutes mapID [randSeed]" << endl;
      exit(1);
   }

   int nbrRoutes = atoi(argv[1]);
   uint16 mapID  = atoi(argv[2]);

   if (argc == 4)
      srand(atoi(argv[3]));
   else
      srand(14);

   RoutingMap* routingMap = new RoutingMap(mapID);
   CalcRoute*  calc       = new CalcRoute(routingMap);

   // Connect to MM and load map
   routingMap->load();

   FILE* file;
   file = fopen("randomnodes","w");

   RoutingNode *origin, *destination;
   for (int i = 0; i < nbrRoutes; i++) {
      uint32 itemID = routingMap->getRandomItemID();
      origin = routingMap->getNodeFromTrueNodeNumber(itemID);
      do{
         itemID = routingMap->getRandomItemID();
         destination = routingMap->getNodeFromTrueNodeNumber(itemID);
      }
      while(origin == destination);

         // Stream the result to a file
//    fprintf(file, "%u %u\n", origin->getItemID(), destination->getItemID());
      fprintf(file,
              "%u\t%u\t%u\t%u\n",
              mapID,
              origin->getItemID(),
              mapID,
              destination->getItemID());
   }

   fclose(file);

   delete calc;
} // end_main

#endif
     
      

