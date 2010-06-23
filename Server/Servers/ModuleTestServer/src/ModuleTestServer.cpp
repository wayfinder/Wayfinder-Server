/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MTSender.h"
#include "MTReceiver.h"
#include "MTLog.h"
#include "ISABThread.h"
#include "GfxConstants.h"
#include "CommandlineOptionHandler.h"
#include "NetUtility.h"
#include "Utility.h"

#include <iostream>

// r 0 0 0 1 0 2 1 0 0 0 0 0 0 0 0 0
// s 1 66 "" "" "vipe" 0 0 0 0 0 255

char* readString(char* str)
{
   char   c;
   uint16 i = 0;

   cin >> c;
   while (c != '"')
      cin >> c;
   cin >> c;
   while (c != '"') {
      str[i++] = c;
      cin >> c;
   }
   str[i] = '\0';
   return str;
}


void printHelp()
{
   cout
<< "The following commands are available:"                           << endl
<< "R repetitions delayTime                   Set repetitions, delay (us)"
                                                                     << endl
<< "@ mapID itemID                            RouteItemrequest "   << endl
<< "b                                         AllMapRequestPacket"   << endl
<< "d[abcd]       Session type:                                    " << endl
<< "              a    UserLogin:      da userName password"         << endl
<< "              b    SessionCleanup: db sessionID sessionKey"      << endl
<< "              c    GetUserNavDest: dc onlyUnSentNavDestinations "
      "onlyNavigatorLastContact navID navAddress"                    << endl
<< "              d    AddUserNavDest: dd navID sent created type name senderID lat lon"                                                        << endl
<< "e mapID itemID                            Expand item req.pack." << endl
<< "f mapID itemID                            CoordinateOnItemPacket"<< endl
<< "                                          GfxMap request packet" << endl

<< "l mapID moduleType                        Load map request packet"
                                                                     << endl
<< "                                          mapID in hex"          << endl
<< "                                          moduleType:"           << endl
<< "                                           0    = INVALID"       << endl
<< "                                           1    = MAP"           << endl
<< "                                           2    = ROUTE"         << endl
<< "                                           3    = SEARCH"        << endl
<< "                                           4    = USER"          << endl
<< "                                           5    = SMS"           << endl
<< "                                           7    = Info"          << endl
<< "D mapID moduleType                        Delete map request packet" 
                                                                     << endl
<< "                                          like loadmap"          << endl   
<< "m mapType mapID                           Map request packet"    << endl
<< "  mapType: 0 Search, 1 Route, 2 Gfx, 3 StringTable          "    << endl
<< "n mapID serviceID ip port                 MapSubscReq "          << endl
<< "o nbrLocations locations locationType"                           << endl
<< "  requestedLanguage mapID matching"                              << endl
<< "  stringPart sortingType uniqueOrFull dbMask" << endl
<< "  minNbrHits                             Overview Search Request"<< endl
                                                                     << endl
<< "s \"Email adress\" \"from adr.\" \"subject\"\"datastring\""      << endl
<< "t                                         Test request packet"   << endl
<< "u numMapID [mapID] zipCode city"                                 << endl
<< "  searchString nbrHits matchType"                                << endl
<< "  stringPart sortingType categoryType"                           << endl
<< "  language nbrSortedHits editDistanceCutoff dbMask"              << endl
<< "  nbrMasks [maskItemIDs]                  User Search request"   << endl
<< "v numMapID [mapID] zipCode city"                                 << endl
<< "  searchString nbrHits matchType"                                << endl
<< "  stringPart sortingType categoryType"                           << endl
<< "  language dbMask                         Vanilla Search request"<< endl
<< "x type                                    Random request packet" << endl
<< "å r|d|x lat lon angle                     Coordinate request packet "
      "[rad|deg|rad32]"                                              << endl
<< "ä \"senderPhone\" \"recipientPhone\" \"datastring\""             << endl
<< "                                          SMS send req. packet"  << endl
<< "ö mapID level                             EdgeNodesReq"          << endl
<< "W waittime                                Wait waittime milliseconds"
                                                                     << endl
<< "E endwaittime                             Waittime for packets at"
                                                                     << endl
<< "                                          end in milliseconds"   << endl
<< "O nbrOutstandingpackets                   The amount of outstanding"
                                                                     << endl
<< "                                          packets"               << endl
<< "T packetType mapID nodeId distID  Traffic Cost Request Packet"   << endl
<< "  packetType:1 AddReqest, :2 RemoveRequest  "   << endl 
<< "  packetType:3 subscribeRequest (mapID, bool subscribe(or uns.))"<< endl 
<< "H                                         Help"                  << endl
<< "Q                                         Quit"                 << endl;
}

void handleOverviewSearchRequest(uint32 rep, uint32 repDelay, MTSender* mts)
{
//   cout << "Entered HandleOverviewSearchRequest()" << endl;
   uint32           nbrLocations;
   char**           locations;
   uint32           locationType;
   uint32           requestedLanguage;            // 0<=x<=5
   uint32           mapID;
   int64            tmpMapID;
   uint16 matching;         // see SearchTypes.h
   uint16 stringPart;                  // see SearchTypes.h
   uint16 sortingType;
   bool             uniqueOrFull;
   uint16           dbMask;
   uint16           minNbrHits;

   cin >> nbrLocations;
   locations = new char*[nbrLocations];
   for (uint32 i = 0; i < nbrLocations; i++){
      locations[i]=new char[128];
   }

   for (uint32 i = 0; i < nbrLocations; i++){
      readString(locations[i]);
   }

   const char** tmpLocations = const_cast<const char**>(locations);

   cin >> locationType >> requestedLanguage >> tmpMapID
       >> matching >> sortingType >> stringPart >> uniqueOrFull
       >> dbMask >> minNbrHits;


   if (tmpMapID < 0){
      mapID=MAX_UINT32;
   }
   else {
      mapID=tmpMapID;
   }


   for (uint32 i=0; i < rep; i++) {
      mts->sendOverviewSearchRequest(
         nbrLocations, 
         tmpLocations, 
         locationType, 
         requestedLanguage, 
         mapID, 
         static_cast<SearchTypes::StringMatching>(matching), 
         static_cast<SearchTypes::StringPart>(stringPart),
         static_cast<SearchTypes::SearchSorting>(sortingType),
         uniqueOrFull, dbMask, minNbrHits);
      if ( repDelay != 0 ){
         JTCThread::sleep(0, repDelay);
      }
   }
   delete [] tmpLocations;
} // handleOverviewSearchRequest(uint32 rep, uint32 repDelay, MTSender* mts)


int main(int argc, char *argv[])
{
   CommandlineOptionHandler coh(argc, argv);
   coh.setSummary("");

   bool compactOutput = false;
   coh.addOption("-r", "--compact",
                 CommandlineOptionHandler::presentVal,
                 0, &compactOutput, "F",
                 "Set compact output mode");
   if(!coh.parse()) {
      cerr << "ModuleTestServer: Error on commandline (-h for help)" << endl;
      exit(1);
   }
   if(!Properties::setPropertyFileName(coh.getPropertyFileName())) {
      cerr << "No such file or directory: '"
           << coh.getPropertyFileName() << "'" << endl;
      exit(1);
   }

   ISABThreadInitialize isabThreadInitialize;
   MTLog* log = new MTLog("logfil", compactOutput);
   MTReceiver* mtr = new MTReceiver(log);
   MTSender* mts = new MTSender(mtr->getPort(), log);
   mtr->start();

   // Beginning of main loop
   bool cont = true;
   char command;
   uint32 packetWaitTime = 8000;//0;
   uint32 rep = 1;
   uint32 repDelay = 0;
   printHelp();

   while (cont && cin) {
      //cout << "# ";
      cin >> command;
      switch (command) {
         case 'R': {
            cin >> rep >> repDelay;
         }
         break;

         case 'x': {
            cout << "Not yet implemented" << endl;
         }
         break;

         case 'm': {
            uint32 mapID;
            uint16 tempMapType;
            byte mapType;
            cin >> tempMapType >> mapID;
            mapType = tempMapType & MAX_BYTE;
            for (uint32 i=0; i < rep; i++) {
               mts->sendMapRequest(mapType, mapID);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
         }
         break;

         case 'n': {
            MC2String s_mapID; // Hex will work
            cin >> s_mapID;
            MC2String s_serviceID;
            cin >> s_serviceID;
            MC2String s_ip;
            cin >> s_ip;            
            uint32 port;
            cin >> port;
            uint32 mapID = strtoul(s_mapID.c_str(), NULL, 0);
            uint32 ip    = NetUtility::iptoh(s_ip.c_str());
            uint32 serviceID = strtoul(s_serviceID.c_str(), NULL, 0);
            mts->sendMapSubscriptionRequestPacket(mapID, ip, port, serviceID);
         }
         break;
         case 't': {
            uint32 mapID;
            cin >> mapID;
            for (uint32 i=0; i < rep; i++) {
               mts->sendTestRequest(mapID);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
         }
         break;

         case 'b': {
            for (uint32 i=0; i < rep; i++) {
               mts->sendAllMapRequest();
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
         }
         break;

         case 'l': {
            uint32 mapID;
            uint32 moduleType = 0;
            cin >> hex;
            cin >> mapID;
            cin >> dec;
            cin >> moduleType;
            for (uint32 i=0; i < rep; i++) {
               mts->sendLoadMapRequest(mapID, (moduletype_t)moduleType);
               if ( repDelay != 0 ) {
                  JTCThread::sleep(0, repDelay);
               }
            }
         }
         break;

         case 'D': {
            uint32 mapID;
            uint32 moduleType = 0;
            cin >> hex;
            cin >> mapID;
            cin >> dec;
            cin >> moduleType;
            for (uint32 i=0; i < rep; i++) {
               mts->sendDeleteMapRequest(mapID, (moduletype_t)moduleType);
               if ( repDelay != 0 ) {
                  JTCThread::sleep(0, repDelay);
               }
            }
         }
         break;

         case 'å': {
            char unit;
            cin >> unit;
            int32 lat, lon;
            if ( unit == 'd' ) {
               double dLat, dLon;
               cin >> dLat >> dLon;
               lat = (uint32) ( dLat * GfxConstants::degreeFactor);
               lon = (uint32) ( dLon * GfxConstants::degreeFactor);
            } else if ( unit == 'r' ) {
               double rLat, rLon;
               cin >> rLat >> rLon;
               lat = (uint32) ( rLat * GfxConstants::radianFactor);
               lon = (uint32) ( rLon * GfxConstants::radianFactor);
               cout << "lat = " << lat << ", lon = " << lon << endl;
               cout << "lat = " << lat/GfxConstants::degreeFactor 
                    << " deg, lon = " << lon / GfxConstants::degreeFactor 
                    << " deg" << endl;
            } else {
               // Rad32 (default)
               cin >> lat >> lon;
            }
            uint16 angle;
            cin >> angle;
            for (uint32 i=0; i < rep; i++) {
               mts->sendCoordinateRequest(lat, lon, angle);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
         }
         break;

         case 'ä': {
            char* lineBuffer = new char[768];
            char* senderPhone = new char[256];
            char* recipientPhone = new char[256];
            char* data = new char[256];
            cin.getline(lineBuffer, 768);
            char *p;
            p = Utility::getString(lineBuffer, '"', '"', senderPhone, 256);
            p = Utility::getString(p, '"', '"', recipientPhone, 256);
            p = Utility::getString(p, '"', '"', data, 256);
            //cout << "Du skrev senderPhone=" << senderPhone << " rec = "
            //     << recipientPhone << " data = " << data << endl;
            for (uint32 i=0; i < rep; i++) {
               mts->sendSMSRequest(senderPhone, recipientPhone, data);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }            
            delete [] data;
            delete [] recipientPhone;
            delete [] senderPhone;
            delete [] lineBuffer;
         }
         break;

         case 'ö': {
            uint32 mapID;
            int level;
            cin >> mapID;
            cin >> level;
            mts->sendEdgeNodesRequest(mapID,level);
         }
         break;

         case '@': {
            uint32 mapID;
            uint32 itemID;
            cin >> mapID;
            cin >> itemID;
            mts->sendRouteExpandItemRequest(mapID, itemID);
         }
         break;

         case 's': {
            char* lineBuffer = new char[768];
            // Email adress and text
            char* adress = new char[256];
            char* fromAdress = new char[256];
            char* subject = new char[256];
            char* data = new char[256];
            cin.getline(lineBuffer, 768);
            char *p;
            p = Utility::getString(lineBuffer, '"', '"', adress, 256);
            p = Utility::getString(p, '"', '"', fromAdress, 256);
            p = Utility::getString(p, '"', '"', subject, 256);
            p = Utility::getString(p, '"', '"', data, 256);
            for (uint32 i=0; i < rep; i++) {
               mts->sendEmailRequest(adress, fromAdress, subject, data);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }            
            delete [] adress;
            delete [] data;
         }
         break;
         
         case 'T': {
            int packetType;
            uint32 mapID;
            uint32 nodeID;
            uint32 distID;
            uint32 datexID;
            cin >> packetType >> mapID >> nodeID >> distID >> datexID;
            switch (packetType){
               case 1:
                  mts->sendAddDisturbanceRequest(mapID, nodeID, distID);
                  break;
               case 2:
                  mts->sendRemoveDisturbanceRequest(datexID);
                  break;
               case 3:
                  mts->sendDisturbanceSubscriptionRequest(mapID,
                                                          (bool)nodeID,
                                                          MAX_BYTE);
                  break;
               default:{
                     cerr << "Unrecognized option: '" << packetType << endl;
                  }
                  break;
            }
            if ( repDelay != 0 )
               JTCThread::sleep(0, repDelay);
            
         }
         break;            
            
         case 'e': {
            uint32 itemID;
            uint32 mapID;
            cin >> mapID >> itemID;
            for (uint32 i=0; i < rep; i++) {
               mts->sendExpandItemRequest(mapID, itemID);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
         }
         break;

         case 'v': {
            uint32   numMapID;
            uint32*  mapIDs;
            char     zipCode[32];
            char     city[32];
            char     searchString[128];
            uint16   tmpNbrHits;
            uint8    nbrHits;
            int      matchType;
            int      stringPart;
            int      sortingType;
            uint16   categoryType;
            uint16   language;
            uint16   dbMask;

            cin >> numMapID;
            mapIDs = new uint32[numMapID];
            for (uint32 i = 0; i < numMapID; i++)
               cin >> mapIDs[i];
            readString(zipCode);
            readString(city);
            readString(searchString);
            cin >> tmpNbrHits >> matchType >> stringPart
                >> sortingType >> categoryType >> language >> dbMask;
            nbrHits = tmpNbrHits & 0xff;

            for (uint32 i=0; i < rep; i++) {
               mts->sendVanillaSearchRequest(
                  numMapID, mapIDs, zipCode,
                  city, searchString, nbrHits,
                  static_cast<SearchTypes::StringMatching>(matchType),
                  static_cast<SearchTypes::StringPart>(stringPart),
                  static_cast<SearchTypes::SearchSorting>(sortingType),
                  categoryType, language, dbMask);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
            delete [] mapIDs;
         }
         break;

         case 'u': {
            uint32   numMapID;
            uint32*  mapIDs;
            char     zipCode[32];
            char     city[32];
            char     searchString[128];
            uint16   tmpNbrHits;
            uint8    nbrHits;
            int      matchType;
            int      stringPart;
            int      sortingType;
            uint16   categoryType;
            uint16   language;
            uint16   tmpNbrSortedHits;
            uint8    nbrSortedHits;
            uint16   editDistanceCutoff;
            uint16   dbMask;
            uint32   nbrMasks;

            cin >> numMapID;
            mapIDs = new uint32[numMapID];
            for (uint32 i = 0; i < numMapID; i++)
               cin >> mapIDs[i];
            readString(zipCode);
            readString(city);
            readString(searchString);
            cin >> tmpNbrHits >> matchType >> stringPart
                >> sortingType >> categoryType >> language
                >> tmpNbrSortedHits >> editDistanceCutoff
                >> dbMask >> nbrMasks;
            cerr << "nbrMasks = " << nbrMasks << endl;
            uint32* maskItemIDs = new uint32[nbrMasks];
            for (uint8 i = 0; i < nbrMasks; i++) {
               cin >> maskItemIDs[i];
            }
            nbrHits = tmpNbrHits & 0xff;
            nbrSortedHits = tmpNbrSortedHits & 0xff;

            for (uint32 i=0; i < rep; i++) {
               mts->sendUserSearchRequest(
                  numMapID, mapIDs, zipCode, city,
                  searchString, nbrHits,
                  static_cast<SearchTypes::StringMatching>(matchType),
                  static_cast<SearchTypes::StringPart>(stringPart),
                  static_cast<SearchTypes::SearchSorting>(sortingType),
                  categoryType, language, nbrSortedHits, editDistanceCutoff,
                  dbMask, nbrMasks, maskItemIDs);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
            delete maskItemIDs;
            delete [] mapIDs;
         }
         break;
         case 'o': {
            handleOverviewSearchRequest(rep, repDelay, mts);
         }
         break;

         case 'f': {
            uint32 itemID;
            uint32 mapID;
            cin >> mapID >> itemID;
            for (uint32 i=0; i < rep; i++) {
               mts->sendCoordinateOnItemRequest(mapID, itemID);
               if ( repDelay != 0 )
                  JTCThread::sleep(0, repDelay);
            }
         }
         break;


         case 'd': {
            cout << "d" << endl;
            cin >> command;
            switch (command) {
               case 'a': {
                  char lineBufferchar[768];
                  char login[256];
                  char passwd[256];
                  char* p = NULL;

                  cin.getline( lineBufferchar, 768 );
                  p = lineBufferchar;
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', login, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', '\0', passwd, 255 );
                  DEBUG8(cerr << "da " << login << " " << passwd << endl;);
                  for (uint32 i=0; i < rep; i++) {
                     mts->sendUserLoginRequest( login, passwd );
                     if ( repDelay != 0 )
                        JTCThread::sleep(0, repDelay);
                  }
               }
               break;
               case 'b': {
                  char lineBufferchar[768];
                  char sessionID[256];
                  char sessionKey[256];
                  
                  char* p = NULL;

                  cin.getline( lineBufferchar, 768 );
                  p = lineBufferchar;
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', sessionID, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', '\0', sessionKey, 255 );
                  
                  /*DEBUG8(*/cerr << "db " << sessionID << " " 
                                  << sessionKey << endl;/*)*/;
                  for (uint32 i=0; i < rep; i++) {
                     mts->sendSessionCleanUpRequest( sessionID, 
                                                     sessionKey );
                     if ( repDelay != 0 )
                        JTCThread::sleep(0, repDelay);
                  } 
               }
               break;
               case 'c' : {
                  char lineBufferchar[768];
                  char onlyUnsent[256];
                  char onlyContact[256];
                  char navID[256];      
                  char navAddress[256];
                  char* p = NULL;

                  cin.getline( lineBufferchar, 768 );
                  p = lineBufferchar;
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', onlyUnsent, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', onlyContact, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', '\0', navID, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', '\0', navAddress, 255 );
                  
                  for (uint32 i=0; i < rep; i++) {
                     mts->sendGetUserNavDestinationRequest( onlyUnsent, 
                                                            onlyContact,
                                                            navID,
                                                            navAddress );
                     if ( repDelay != 0 )
                        JTCThread::sleep(0, repDelay);
                  }  
               }
               break;
               case 'd' : {
                  char lineBufferchar[768];
                  char navID[256];
                  char sent[256];
                  char created[256];
                  char type[256];
                  char name[256];
                  char senderID[256];
                  char lat[256];
                  char lon[256];
                  char* p = NULL;

                  cin.getline( lineBufferchar, 768 );
                  p = lineBufferchar;
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', navID, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', sent, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', created, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', type, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', name, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', senderID, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', ' ', lat, 255 );
                  while ( *p == ' ' ) p++;
                  p = Utility::getString( p, '\0', '\0', lon, 255 );
                  
                  for (uint32 i=0; i < rep; i++) {
                     mts->sendAddUserNavDestinationRequest( navID, 
                                                            sent,
                                                            created,
                                                            type,
                                                            name,
                                                            senderID,
                                                            lat,
                                                            lon );
                     if ( repDelay != 0 )
                        JTCThread::sleep(0, repDelay);
                  }  
               }
               break;
            }
         }
         break;

         case 'W': {
            uint32 waitTime;

            cin >> waitTime;
            JTCThread::sleep(waitTime);
         }
         break;

         case 'E': {
            cin >> packetWaitTime;
            cout << "New packet wait time " << packetWaitTime << endl;
         }
         break;

         case 'O': {
            uint32 size;
            cin >> size;
            log->setReceiveBufferSize(size);
            cout << "New receivebuffer size " << log->getReceiveBufferSize() 
                 << endl;
         }
         break;

         case 'H': {
            printHelp();
         }
         break;

         case 'Q': {
            cont = false;
         }
         break;

         default: {
            cerr << "Unrecognized command: '" << command 
                 << "' (" << (int)command << ")" << endl;
         }
         break;
      }
      
   }
   // End of main loop

   if ( !cin ) {
      cerr << "ModuleTestServer::main cin is not ok." << endl;
   }

   // Wait for packets to return 
   uint32 time = TimeUtility::getRealTime();
   DEBUG8(cerr << log->getNbrWaiting() << " packets waiting" << endl;);
   while ( (log->getNbrWaiting() > 0) && 
           ((time - mtr->getLastPacketTimeStamp())*1000 < packetWaitTime) ){
      JTCThread::sleep(20);
      time = TimeUtility::getRealTime();
   }

   mtr->terminate();
   isabThreadInitialize.waitTermination();
//   DEBUG8(cerr << "About to delete mts" << endl;);
   delete mts;
   //delete mtr;
   DEBUG8(cerr << "About to delete log" << endl;);
   delete log;
   DEBUG8(cerr << "Deleted log" << endl;);
   return 0;
}
