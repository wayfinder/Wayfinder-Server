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

#include "SMSPacket.h"
#include "ExpandItemPacket.h" 
#include "EdgeNodesPacket.h"
#include "OrigDestInfo.h"
#include "RouteExpandItemPacket.h"
#include "MapSubscriptionPacket.h"
#include "ItemIDTree.h"

#include "NetUtility.h"

uint32 MTSender::mapModuleIP = 0;
uint32 MTSender::routeModuleIP = 0;
uint32 MTSender::searchModuleIP = 0;
uint32 MTSender::testModuleIP = 0;
uint32 MTSender::userModuleIP = 0;
uint32 MTSender::SMSModuleIP = 0;
uint32 MTSender::emailModuleIP = 0;
uint32 MTSender::trafficCostModuleIP = 0;
uint32 MTSender::gfxModuleIP = 0;

uint16 MTSender::mapModulePort = 0;
uint16 MTSender::routeModulePort = 0;
uint16 MTSender::searchModulePort = 0;
uint16 MTSender::testModulePort = 0;
uint16 MTSender::userModulePort = 0;
uint16 MTSender::SMSModulePort = 0;
uint16 MTSender::emailModulePort = 0;
uint16 MTSender::trafficCostModulePort = 0;
uint16 MTSender::gfxModulePort = 0;

MTSender::MTSender(uint16 port, MTLog* log)
{

   mapModuleIP = MultiCastProperties::getNumericIP( MODULE_TYPE_MAP, true );
   routeModuleIP =
      MultiCastProperties::getNumericIP( MODULE_TYPE_ROUTE, true );
   searchModuleIP =
      MultiCastProperties::getNumericIP( MODULE_TYPE_SEARCH, true );
   userModuleIP =
      MultiCastProperties::getNumericIP( MODULE_TYPE_USER, true );
   SMSModuleIP =
      MultiCastProperties::getNumericIP( MODULE_TYPE_SMS, true );
   emailModuleIP =
      MultiCastProperties::getNumericIP( MODULE_TYPE_SMTP, true );
   trafficCostModuleIP =
      MultiCastProperties::getNumericIP( MODULE_TYPE_TRAFFIC, true );
   gfxModuleIP =
      MultiCastProperties::getNumericIP( MODULE_TYPE_GFX, true );

   mapModulePort = MultiCastProperties::getPort( MODULE_TYPE_MAP, true );
   routeModulePort =
      MultiCastProperties::getPort( MODULE_TYPE_ROUTE, true );
   searchModulePort =
      MultiCastProperties::getPort( MODULE_TYPE_SEARCH, true );
   userModulePort =
      MultiCastProperties::getPort( MODULE_TYPE_USER, true );
   SMSModulePort =
      MultiCastProperties::getPort( MODULE_TYPE_SMS, true );
   emailModulePort =
      MultiCastProperties::getPort( MODULE_TYPE_SMTP, true );
   trafficCostModulePort =
      MultiCastProperties::getPort( MODULE_TYPE_TRAFFIC, true );
   gfxModulePort =
      MultiCastProperties::getPort( MODULE_TYPE_GFX, true );

      
   myIP = NetUtility::getLocalIP();
   UDPSender = new DatagramSender();
   myPort = port;
   this->log = log;
   packID = 0;
   DEBUG1(cerr << "ModuleTestServer listens for packets at " << myIP 
               << "." << myPort << endl);
}


MTSender::~MTSender()
{
   delete UDPSender;
}


void MTSender::sendRandomPacket(byte type)
{
}

void MTSender::sendCoveredIDsRequest(int32 lat, int32 lon,
                                uint32 innerR, uint32 outerR,
                                uint16 startA, uint16 stopA)
{
   char* tmpStr = new char[64];
   packID++;
   sprintf(tmpStr, "CoveredIDsPacket\n");
   log->putRequest(packID, tmpStr);

   CoveredIDsRequestPacket outPacket(NULL, packID, 0, 0);
   outPacket.setLat(lat);
   outPacket.setLon(lon);
   outPacket.setOuterRadius(outerR);
   outPacket.setInnerRadius(innerR);
   outPacket.setStartAngle(startA);
   outPacket.setStopAngle(stopA);

   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   cerr << "Sending CoveredIDsPacket!" << endl;
   UDPSender->send(&outPacket, mapModuleIP, mapModulePort);
   if (packID % 1000 == 0)
      cout << "Sent CoveredIDsPacket with packetID = " << packID 
           << endl;
} // MTSender::sendCoveredIDsRequest(...)


void MTSender::sendAllMapRequest()
{
   MC2INFO("MTSender::sendAllMapRequest()");
   char* tmpStr = new char[64];
   packID++;
   strcpy(tmpStr, "AllMapRequestPacket\n");
   log->putRequest(packID, tmpStr);
   
   AllMapRequestPacket* outPacket =
      new AllMapRequestPacket(AllMapRequestPacket::ONLY_MAPID);
   outPacket->setOriginPort(myPort);
   outPacket->setOriginIP(myIP);
  
   UDPSender->send(outPacket, mapModuleIP, mapModulePort);

   if (packID % 1000 == 0 || packID == 1)
      cout << "Sent AllMapRequestPacket with packetID = " << packID 
           << endl;
   
   delete outPacket;
} // MTSender::sendAllMapRequest(...)

void MTSender::sendCoordinateRequest(int32 lat, int32 lon, uint16 angle)
{
   char* tmpStr = new char[64];
   packID++;
   strcpy(tmpStr, "CoordinateRequestPacket\n");
   log->putRequest(packID, tmpStr);
   
   CoordinateRequestPacket* outPacket =
      new CoordinateRequestPacket(packID, 0, lat, lon, myIP, myPort, angle);
  
   outPacket->addItemType(ItemTypes::streetSegmentItem);
   
   UDPSender->send(outPacket, mapModuleIP, mapModulePort);

   if (packID % 1000 == 0 || packID == 1)
      cout << "Sent CoordinateRequestPacket with packetID = " << packID 
           << endl;

   
   delete outPacket;
} // MTSender::sendCoordinateRequest(...)


void MTSender::sendSMSRequest(char* senderPhone, char* recipientPhone,
                    char* data)
{
   char* tmpStr = new char[128];
   packID++;
   sprintf(tmpStr, "SMSSendRequestPacket\n");
   log->putRequest(packID, tmpStr);

   SMSSendRequestPacket outPacket(packID, 0, myIP, myPort);
   outPacket.fillPacket(CONVERSION_TEXT, senderPhone, recipientPhone,
                        strlen(data)+1, (byte*)data);
   UDPSender->send(&outPacket, SMSModuleIP, SMSModulePort);
   if ( packID % 1000 == 0)
      cout << "Send packet ID =" << packID << endl;
   
} // MTSender::sendSMSRequest
   
void MTSender::sendEmailRequest(char* adress, char* fromAdress, 
                                char* subject, char* data)
{
   cerr << "Sending email request" << endl 
        << "   adress=" << adress << endl 
        << "   fromAdress=" << fromAdress << endl 
        << "   subject=" << subject << endl
        << "   data=" << data << endl;
   char* tmpStr = new char[128];
   packID++;
   sprintf(tmpStr, "EmailRequestPacket\n");
   log->putRequest(packID, tmpStr);

   SendEmailRequestPacket outPacket;
   outPacket.setPacketID( packID );
   outPacket.setRequestID( 0 );
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   outPacket.setData(adress, fromAdress, subject, data);
   cerr << "emailModuleIP = " << emailModuleIP 
        << ", emailModulePort = " << emailModulePort << endl;
   UDPSender->send(&outPacket, emailModuleIP, emailModulePort);
   if ( packID % 1000 == 0)
      cout << "Send packet ID =" << packID << endl;
   
} // MTSender::sendEmailRequest
   
 
void MTSender::sendTestRequest(uint32 mapID)
{
   char* tmpStr = new char[64];
   packID++;
   sprintf(tmpStr, "TestRequestPacket\n");
   log->putRequest(packID, tmpStr);

   TestRequestPacket outpacket(mapID,
                               TimeUtility::getCurrentMicroTime(),
                               myIP, myPort);
   outpacket.setPacketID(packID);
   UDPSender->send(&outpacket, testModuleIP, testModulePort);
   if (packID % 1000 == 0)
      cout << "Sent packet ID = " << packID << endl;
} // MTSender::sendTestRequest(...)



void MTSender::sendLoadMapRequest( uint32 mapID, moduletype_t moduleType )
{
   char* tmpStr = new char[64];
   packID++;
   sprintf(tmpStr, "LoadMapRequestPacket\n"
           "   mapID      = %u\n"
           "   moduleType = %u\n", 
           mapID, moduleType);
   log->putRequest(packID, tmpStr);

   LoadMapRequestPacket outpacket(mapID, myIP, myPort);
   outpacket.setPacketID(packID);
   uint32 ip;
   uint16 port;
   switch ( moduleType ) {
      case MODULE_TYPE_ROUTE :
         DEBUG2(cerr << "Module type route" << endl);
         ip = routeModuleIP;
         port = routeModulePort;
         break;
      case MODULE_TYPE_MAP :
         DEBUG2(cerr << "Module type map" << endl);
         ip = mapModuleIP;
         port = mapModulePort;
         break;
      case MODULE_TYPE_SEARCH :
         DEBUG2(cerr << "Module type search" << endl);
         ip = searchModuleIP;
         port = searchModulePort;
         break;
      case MODULE_TYPE_USER :
         DEBUG2(cerr << "Module type user" << endl;);
         ip = userModuleIP;
         port = userModulePort;
         break;
      case MODULE_TYPE_SMS:
         DEBUG2(cerr << "Module type SMS" << endl;);
         ip = SMSModuleIP;
         port = SMSModulePort;
         break;
      case MODULE_TYPE_TRAFFIC:
         DEBUG2(cerr << "Module type traffic" << endl;);
         ip = trafficCostModuleIP;
         port = trafficCostModulePort;
         break;
      default :
         DEBUG1(cerr << "Module type UNKNOWN, using test" << endl);
         ip = testModuleIP;
         port = testModulePort;
         break;
   }

   UDPSender->send(&outpacket, ip, port);
   cout << "Sent packet ID = " << packID << endl;
} // MTSender::sendLoadMapRequest(...)

void MTSender::sendDeleteMapRequest( uint32 mapID, moduletype_t moduleType )
{
   char* tmpStr = new char[64];
   packID++;
   sprintf(tmpStr, "DeleteMapRequestPacket\n"
           "   mapID      = %u\n"
           "   moduleType = %u\n", 
           mapID, moduleType);
   log->putRequest(packID, tmpStr);

   DeleteMapRequestPacket outpacket(mapID, myIP, myPort);
   outpacket.setPacketID(packID);
   uint32 ip;
   uint16 port;
   switch ( moduleType ) {
      case MODULE_TYPE_ROUTE :
         DEBUG2(cerr << "Module type route" << endl);
         ip = routeModuleIP;
         port = routeModulePort;
         break;
      case MODULE_TYPE_MAP :
         DEBUG2(cerr << "Module type map" << endl);
         ip = mapModuleIP;
         port = mapModulePort;
         break;
      case MODULE_TYPE_SEARCH :
         DEBUG2(cerr << "Module type search" << endl);
         ip = searchModuleIP;
         port = searchModulePort;
         break;
      case MODULE_TYPE_USER :
         DEBUG2(cerr << "Module type user" << endl;);
         ip = userModuleIP;
         port = userModulePort;
         break;
      case MODULE_TYPE_SMS:
         DEBUG2(cerr << "Module type SMS" << endl;);
         ip = SMSModuleIP;
         port = SMSModulePort;
         break;
      case MODULE_TYPE_TRAFFIC:
         mc2dbg2 << "Module type traffic" << endl;
         ip   = trafficCostModuleIP;
         port = trafficCostModulePort;
         break;
      default :
         DEBUG1(cerr << "Module type UNKNOWN, using test" << endl);
         ip = testModuleIP;
         port = testModulePort;
         break;
   }

   UDPSender->send(&outpacket, ip, port);
   cout << "Sent packet ID = " << packID << endl;
} // MTSender::sendDeleteMapRequest(...)


void MTSender::sendMapRequest(byte mapType, uint32 mapID)
{
   char* tmpStr = new char[64];
   packID++;
   sprintf(tmpStr, "MapRequestPacket\n"
                   "   mapType = %u\n"
                   "   mapID   = %u\n", mapType, mapID);
   log->putRequest(packID, tmpStr);

   MapRequestPacket outpacket(0, 0, mapType, mapID);
   outpacket.setOriginIP(myIP);
   outpacket.setOriginPort(myPort);
   outpacket.setPacketID(packID);
   outpacket.setRequestID(mapType);
   UDPSender->send(&outpacket, mapModuleIP, mapModulePort);
   cout << "Sent packet ID = " << packID << endl;
} // MTSender::sendMapRequest(...)

void 
MTSender::sendCoordinateOnItemRequest(uint32 mapID, uint32 itemID)
{
   char* tmpStr = new char[128];
   packID++;
   sprintf(tmpStr, "CoordinateOnItemRequestPacket\n"
                   "   mapID  = %u\n"
                   "   itemID = %u\n", mapID, itemID);
   log->putRequest(packID, tmpStr);
                                                    // send bbox
   CoordinateOnItemRequestPacket outpacket(packID, 0, true);
   outpacket.setOriginIP(myIP);
   outpacket.setOriginPort(myPort);
   outpacket.setMapID(mapID);
   outpacket.add(itemID);
   UDPSender->send(&outpacket, mapModuleIP, mapModulePort);
   cout << "Sent packet ID = " << packID << endl;
} // MTSender:sendCoordinateOnItemRequest(...)



void 
MTSender::sendExpandItemRequest(uint32 mapID, uint32 itemID)
{
   char* tmpStr = new char[64];
   packID++;
   sprintf(tmpStr, "ExpandItemRequestPacket\n"
                   "   mapID  = %u\n"
                   "   itemID = %u\n", mapID, itemID);
   log->putRequest(packID, tmpStr);

   ExpandItemRequestPacket outpacket(packID, 0, mapID);
   outpacket.setItemID(itemID);
   outpacket.setOriginIP(myIP);
   outpacket.setOriginPort(myPort);
   outpacket.setPacketID(packID);
   UDPSender->send(&outpacket, mapModuleIP, mapModulePort);
   cout << "Sent packet ID = " << packID << endl;
} // MTSender:sendExpandItemRequest(...)


void MTSender::sendVanillaSearchRequest(
   uint32   numMapID,
   uint32*  mapID,
   char*    zipCode,
   char*    city,
   char*    searchString,
   uint8    nbrHits,
   SearchTypes::StringMatching matchType,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   uint16   categoryType,
   uint16   language,
   uint8    dbMask )
{
   char* tmpStr = new char[30*13+strlen(zipCode)+strlen(city)+
                           strlen(searchString)];
   packID++;
   sprintf(tmpStr,   "VanillaSearchRequestPacket:\n"
                     "   numMapID     = %u\n"
                     "   mapID        =",
                     numMapID);
   for (uint32 i = 0; i < numMapID; i++)
      sprintf(tmpStr+strlen(tmpStr), " %u", mapID[i]);
   sprintf(tmpStr+strlen(tmpStr),
           "\n   zipCode      = %s\n"
           "   city         = %s\n"
           "   searchString = %s\n"
           "   nbrHits      = %u\n"
           "   matchType    = %u\n"
           "   stringPart   = %u\n"
           "   sortingType  = %u\n"
           "   categoryType = %u\n"
           "   language     = %u\n"
           "   dbMask       = %u\n",
           zipCode,
           city,
           searchString,
           nbrHits,
           matchType,
           stringPart,
           sortingType,
           categoryType,
           language,
           dbMask);
   log->putRequest(packID, tmpStr);

   VanillaSearchRequestPacket outpacket(packID, 0);
   outpacket.setOriginIP(myIP);
   outpacket.setOriginPort(myPort);
   outpacket.encodeRequest(numMapID,
                           mapID,
                           zipCode,
                           0,
                           NULL,
                           0,
                           0, NULL,
                           0,
                           NULL,
                           NULL,
                           NULL,
                           dbMask,
                           searchString,
                           nbrHits,
                           matchType,
                           stringPart,
                           sortingType,
                           categoryType,
                           LangTypes::language_t(language),
                           0,
                           StringTable::SWEDEN_CC ); // STORKA
   UDPSender->send(&outpacket, searchModuleIP, searchModulePort);
   cout << "Sent packet ID = " << packID << endl;
} // MTSender:sendVanillaSearchRequest(...)

void MTSender::sendUserSearchRequest(
   uint32   numMapID,
   uint32*  mapID,
   char*    zipCode,
   char*    city,
   char*    searchString,
   uint8    nbrHits,
   SearchTypes::StringMatching matchType,
   SearchTypes::StringPart stringPart,
   SearchTypes::SearchSorting sortingType,
   uint16   categoryType,
   uint16   language,
   uint8    nbrSortedHits,
   uint16   editDistanceCutoff,
   uint8    dbMask,
   uint8 nbrMasks,
   uint32* maskItemIDs)
{
   char* tmpStr = new char[30*13+strlen(zipCode)+strlen(city)+
                           strlen(searchString)];
   char* tmpStrP = tmpStr;
   packID++;
   tmpStrP += sprintf(tmpStr,   "UserSearchRequestPacket\n"
                      "   numMapID     = %u\n"
                      "   mapID        =",
                      numMapID);
   for (uint32 i = 0; i < numMapID; i++) {
      sprintf(tmpStr+strlen(tmpStr), " %u", mapID[i]);
   }
   tmpStrP += sprintf(tmpStr+strlen(tmpStr),
                      "\n   zipCode       = %s\n"
                      "   city          = %s\n"
                      "   searchString  = %s\n"
                      "   nbrHits       = %u\n"
                      "   matchType     = %u\n"
                      "   stringPart    = %u\n"
                      "   sortingType   = %u\n"
                      "   categoryType  = %u\n"
                      "   language      = %u\n"
                      "   nbrSortedHits = %u\n"
                      "   editDistanceCutoff = %u\n"
                      "   dbMask        = %u\n"
                      "   nbrMasks      = %u\n"
                      "   maskItemIDs:\n",
                      zipCode,
                      city,
                      searchString,
                      nbrHits,
                      matchType,
                      stringPart,
                      sortingType,
                      categoryType,
                      language,
                      nbrSortedHits,
                      editDistanceCutoff,
                      dbMask,
                      nbrMasks);
   for (uint32 i = 0; i < nbrMasks; i++) {
      tmpStrP += sprintf(tmpStrP,
                         "number %u: %u\n",
                         i, maskItemIDs[i]);
   }
   log->putRequest(packID, tmpStr);

   // init
   uint32* masks = new uint32[nbrMasks];
   char** maskNames = new char*[nbrMasks];
   for (uint32 i = 0; i < nbrMasks; i++) {
      masks[i] = 0; // matches everything
      maskNames[i] = new char[1];
      maskNames[i][0] = '\0'; // just a null terminated string.
   }
   
   UserSearchRequestPacket outpacket(packID, 0);
   outpacket.setOriginIP(myIP);
   outpacket.setOriginPort(myPort);
   outpacket.encodeRequest(numMapID,
                           mapID,
                           zipCode,
                           0, NULL,
                           0,
                           0, NULL,
                           nbrMasks, masks, maskItemIDs,
                           (const char**) maskNames,
                           dbMask,
                           searchString,
                           nbrHits,
                           matchType,
                           stringPart,
                           sortingType,
                           categoryType,
                           LangTypes::language_t(language),
                           nbrSortedHits,
                           editDistanceCutoff, 0,
                           set<uint16>(), // categories
                           StringTable::SWEDEN_CC ); // STORKA
   UDPSender->send(&outpacket, searchModuleIP, searchModulePort);
   delete [] masks;
   delete[] maskNames;
   cout << "Sent packet ID = " << packID << endl;
} // MTSender:sendUserSearchRequest(...)



void MTSender::sendOverviewSearchRequest(
   uint32           nbrLocations, 
   const char**     locations, 
   uint32           locationType, 
   uint32           requestedLanguage, 
   uint32           mapID, 
   SearchTypes::StringMatching matching, 
   SearchTypes::StringPart     stringPart,
   SearchTypes::SearchSorting sortingType,
   bool             uniqueOrFull,
   uint8            dbMask,
   uint8            minNbrHits ) 
{
   char* tmpStr = new char[28+8*24+7*10+nbrLocations*128+4096];
   packID++;
   sprintf(tmpStr,   "OverviewSearchRequestPacket\n"
                     "   nbrLocations      = %u\n"
                     "   locations         =",
                     nbrLocations);
   if (nbrLocations <= 0){
      sprintf(tmpStr+strlen(tmpStr), "\n");    
   }
   else {
      sprintf(tmpStr+strlen(tmpStr), " %s\n", locations[0]);
      if (nbrLocations > 1){
         for (uint32 i = 1; i < nbrLocations; i++){
            sprintf(tmpStr+strlen(tmpStr),
                    "                       %s\n",
                    locations[i]);
         }
      }
   }
   sprintf(tmpStr+strlen(tmpStr),
           "   locationType      = %u\n"
           "   requestedLanguage = %u\n"
           "   mapID             = %u\n"
           "   matching          = %u\n"
           "   stringPart        = %u\n"
           "   sortingType       = %u\n"
           "   uniqueOrFull      = %u\n"
           "   dbMask            = %u\n"
           "   minNbrHits        = %u\n",
           locationType,
           requestedLanguage,
           mapID,
           matching,
           stringPart,
           sortingType,
           uniqueOrFull,
           dbMask,
           minNbrHits);


   log->putRequest(packID, tmpStr);

   ItemIDTree itemTree;
   OverviewSearchRequestPacket outpacket(packID, 
                                         0,
                                         nbrLocations, 
                                         locations, 
                                         locationType, 
                                         requestedLanguage, 
                                         mapID, 
                                         matching, 
                                         stringPart,
                                         sortingType,
                                         uniqueOrFull,
                                         dbMask,
                                         minNbrHits,
                                         0,
                                         itemTree);

   outpacket.setOriginIP(myIP);
   outpacket.setOriginPort(myPort);
   UDPSender->send(&outpacket, searchModuleIP, searchModulePort);
   cout << "Sent packet ID = " << packID << endl;
} // MTSender::sendOverviewSearchRequest(...)
  
void
MTSender::sendUserLoginRequest( const char* login, 
                                const char* passwd )
{
   CheckUserPasswordRequestPacket outPacket( ++packID, 0, login, passwd );
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];

   sprintf( tmp, 
            "CheckUserPasswordRequestPacket\n"
            "   Login : %s\n"
            "   Passwd: %s\n",
            login, passwd );
   
   log->putRequest(packID, tmp);   
   
   UDPSender->send(&outPacket, userModuleIP, userModulePort); 
   cout << "Sent packet ID = " << packID << endl;
}


void
MTSender::sendSessionCleanUpRequest( const char* sessionID, 
                                     const char* sessionKey )
{
   SessionCleanUpRequestPacket outPacket( ++packID, 0, 
                                          sessionID, sessionKey);
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];

   sprintf( tmp, 
            "SessionCleanUpRequestPacket\n"
            "   SessionID : %s\n"
            "   SessionKey: %s\n",
            sessionID, sessionKey );
   
   log->putRequest(packID, tmp);   
   
   UDPSender->send(&outPacket, userModuleIP, userModulePort); 
   cout << "Sent packet ID = " << packID << endl;
}


void
MTSender::sendGetUserNavDestinationRequest( const char* onlyUnsent, 
                                            const char* onlyContact,
                                            const char* navID,
                                            const char* navAddress )
{
   bool onlyUnsentVal = ( *onlyUnsent == 't' );
   bool onlyContactVal = (*onlyContact == 't' );
   uint32 navIDVal = strtol( navID, NULL, 10 );

   GetUserNavDestinationRequestPacket outPacket(
      onlyUnsentVal,
      onlyContactVal,
      navIDVal,
      navAddress );
   outPacket.setPacketID( ++packID );
   outPacket.setRequestID( 0 );
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];

   sprintf( tmp, 
            "GetUserNavDestinationRequest\n"
            "   onlyUnSentNavDestinations : %s\n"
            "   onlyNavigatorLastContact: %s\n"
            "   navID: %d\n",
            BP( onlyUnsentVal ), BP( onlyContactVal ), navIDVal );
   
   log->putRequest( packID, tmp );   
   
   UDPSender->send( &outPacket, userModuleIP, userModulePort );
   cout << "Sent packet ID = " << packID << endl;
}


void 
MTSender::sendAddUserNavDestinationRequest( const char* navID, 
                                            const char* sent,
                                            const char* created,
                                            const char* type,
                                            const char* name,
                                            const char* senderID,
                                            const char* lat,
                                            const char* lon )
{
   uint32 navIDVal = strtol( navID, NULL, 10 );
   bool sentVal = (*sent == 't' );
   uint32 createdVal = strtol( created, NULL, 10 );
   uint32 typeVal = strtol( type, NULL, 10 );
   uint32 senderIDVal = strtol( senderID, NULL, 10 );
   uint32 latVal = strtol( lat, NULL, 10 );
   uint32 lonVal = strtol( lon, NULL, 10 );

   DBUserNavDestination nav;
   
   nav.setNavigatorID( navIDVal );
   nav.setSent( sentVal );
   nav.setCreationDate( createdVal );
   nav.setMessageType( UserConstants::navigatorMessageType( typeVal ) );
   nav.setName( name );
   nav.setSenderID( senderIDVal );
   nav.setLat( latVal );
   nav.setLon( lonVal );
   AddUserNavDestinationRequestPacket p( &nav );

   p.setPacketID( ++packID );
   p.setRequestID( 0 );
   p.setOriginIP(myIP);
   p.setOriginPort(myPort);

   char* tmp= new char[1024];

   sprintf( tmp, 
            "AddUserNavDestinationRequest\n"
            "   navigatorID : %d\n"
            "   sent: %s\n"
            "   createdDate: %d\n"
            "   type: %d\n"
            "   name: %s\n"
            "   senderID: %d\n"
            "   lat: %d\n"
            "   lon: %d\n"
            "\n",
            navIDVal,
            BP( sentVal ),
            createdVal,
            typeVal,
            name,
            senderIDVal,
            latVal,
            lonVal );
   
   log->putRequest( packID, tmp );   
   
   UDPSender->send( &p, userModuleIP, userModulePort );
   cout << "Sent packet ID = " << packID << endl;

}


void
MTSender::sendAddDisturbanceRequest(uint32 mapID,
                                    uint32 nodeID,
                                    uint32 distID)
{
/*   if (distID == 0)
      distID = MAX_UINT32;
   AddDisturbanceRequestPacket outPacket( ++packID, 0);
   if (mapID != 2){
      outPacket.encodeRequest(mapID, nodeID,
                              DisturbanceTypes::disturbancetype_t(0),
                              DisturbanceTypes::disturbance_t(0),
                              MAX_INT32,
                              MAX_INT32,
                              MAX_UINT16,                              
                              MAX_UINT32,
                              MAX_UINT32,
                              TimeUtility::getRealTime(),
                              TimeUtility::getRealTime()+120,
                              time(0),
                              "Disturbance info",
                              MAX_UINT32,
                              MAX_UINT32,
                              false,
                              MAX_UINT32,
                              MAX_UINT32);
   }else{
      outPacket.encodeRequest(mapID, nodeID,
                              DisturbanceTypes::disturbancetype_t(0),
                              DisturbanceTypes::disturbance_t(0),
                              MAX_INT32,
                              MAX_INT32,
                              MAX_UINT16,
                              MAX_UINT32,
                              MAX_UINT32,
                              TimeUtility::getRealTime(),
                              TimeUtility::getRealTime()+120,
                              time(0),
                              "Disturbance info",
                              MAX_UINT32,
                              MAX_UINT32,
                              false,
                              MAX_UINT32,
                              MAX_UINT32);
   }
      
   
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];
   
   sprintf( tmp, 
            "AddDistubanceRequestPacket\n"
            "   mapID : %d\n"
            "   nodeID: %d\n"
            "   distID: %d\n",
            mapID, nodeID, distID );
   

   log->putRequest(packID, tmp);   
   
   UDPSender->send(&outPacket, trafficCostModuleIP, trafficCostModulePort); 
   cout << "Sent packet ID = " << packID << endl;
*/
}


void
MTSender::sendRemoveDisturbanceRequest(uint32 datexID)
{
   /*
   RemoveDisturbanceRequestPacket outPacket( ++packID, 0);
   outPacket.encodeRequest(datexID, MAX_UINT32);
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];
   
   sprintf( tmp,
           "RemoveDistubanceRequestPacket\n"
            "   datexID : %d\n"
            ,datexID );
   

   log->putRequest(packID, tmp);   
   
   UDPSender->send(&outPacket, trafficCostModuleIP, trafficCostModulePort); 
   cout << "Sent packet ID = " << packID << endl;
   */
}

void
MTSender::sendDisturbanceSubscriptionRequest(uint32 mapID,
                                             bool subscribe,
                                             byte minRoadSize)
{
   DisturbanceSubscriptionRequestPacket outPacket( ++packID, 0);
   outPacket.encode(mapID, subscribe, minRoadSize);
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];
   sprintf( tmp,
           "DistubanceSubscriptionRequestPacket\n"
            "   mapID : %d\n"
            "   subscribe: %hu\n"
            "   minRoadSize: %hu \n",
            mapID, (int)subscribe, minRoadSize);
   log->putRequest(packID, tmp);   
   UDPSender->send(&outPacket, trafficCostModuleIP, trafficCostModulePort); 
   cout << "Sent packet ID = " << packID << endl;
}

void
MTSender::sendEdgeNodesRequest(uint32 mapID,
                               int level)
{
   OrigDestInfoList emptyDests;
   EdgeNodesRequestPacket outPacket( ++packID,
                                     0,
                                     mapID,
                                     level,
                                     emptyDests,
                                     MAX_UINT32); // All vehicles
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];
   sprintf( tmp,
           "EdgeNodesRequestPacket\n"
            "   mapID : %u\n"
            "   level : %d\n",
            mapID, (int)level);
   log->putRequest(packID, tmp);   
   UDPSender->send(&outPacket, routeModuleIP, routeModulePort);
}


void
MTSender::sendRouteExpandItemRequest(uint32 mapID,
                                     uint32 itemID)
{
   RouteExpandItemRequestPacket outPacket( ++packID,
                                           0);
   outPacket.setOriginIP(myIP);
   outPacket.setOriginPort(myPort);
   char* tmp= new char[1024];
   sprintf( tmp,
           "RouteExpandItemRequestPacket\n"
            "   mapID : %u\n"
            "   itemID : %x\n",
            mapID, itemID);
   log->putRequest(packID, tmp);   
   
   outPacket.add(itemID);
   outPacket.setMapID(mapID);
   UDPSender->send(&outPacket, mapModuleIP, mapModulePort);
}

void
MTSender::sendMapSubscriptionRequestPacket(uint32 mapID,
                                           uint32 ip,
                                           uint32 port,
                                           uint32 serviceID)
{
   map<uint32, uint32> wantedMaps;
   vector<uint32> unwantedMaps;

   wantedMaps.insert(pair<uint32,uint32>(mapID, MAX_UINT32));

   char* tmp= new char[1024];
   sprintf( tmp,
           "MapSubscriptionRequestPacket\n"
            "   mapID : %u\n",
            mapID);
   log->putRequest(packID, tmp);   

   
   MapSubscriptionRequestPacket outPacket(mapID,
                                          serviceID,
                                          wantedMaps,
                                          unwantedMaps);
   outPacket.setOriginIP(ip);
   outPacket.setOriginPort(port);
   UDPSender->send(&outPacket, testModuleIP, testModulePort);
}
