/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GfxConstants.h"
#include "MTReceiver.h"

#include "AllMapPacket.h"
#include "CoordinatePacket.h"
#include "SMSPacket.h"
#include "MapPacket.h"
#include "ExpandItemPacket.h"
#include "RoutePacket.h"
#include "SearchPacket.h"
#include "LoadMapPacket.h"
#include "DeleteMapPacket.h"
#include "TestPacket.h"
#include "UserPacket.h"
#include "SendEmailPacket.h"
#include "AddDisturbancePacket.h"
#include "DisturbanceSubscriptionPacket.h"
#include "UpdateTrafficCostPacket.h"
#include "CoordinateOnItemPacket.h"
#include "UserData.h"
#include "Item.h"
#include "OrigDestInfo.h"
#include "EdgeNodesPacket.h"
#include "SystemPackets.h"
#include "RouteExpandItemPacket.h"
#include "DataBuffer.h"

#include <set>

MTReceiver::MTReceiver(MTLog* log)
{
   UDPReceiver = new DatagramReceiver(DEFAULT_MT_SERVER_PORT,
                                      DatagramReceiver::FINDFREEPORT);
   this->log = log;
   m_packetTimeStamp = TimeUtility::getRealTime();
}


MTReceiver::~MTReceiver()
{
   delete UDPReceiver;
}


uint16 MTReceiver::getPort()
{
   return UDPReceiver->getPort();
}


void MTReceiver::run()
{
   Packet* packet = new Packet(65535);
   while (!terminated) {
      if (UDPReceiver->receive(packet, 1000000)) {
         m_packetTimeStamp = TimeUtility::getRealTime();
         if (packet->getPacketID() % 100 == 0) {
            cout << "Received packet ID = " << packet->getPacketID()
                 << ", subType = " << (uint32) packet->getSubType() << endl;
         }
         switch (packet->getSubType()) {
            case Packet::PACKETTYPE_COORDINATEONITEMREPLY :
               decodeCoordinateOnItemReply(packet);
            break;

            case Packet::PACKETTYPE_MAPREPLY :
               decodeMapReply(packet);
            break;

            case Packet::PACKETTYPE_ROUTEREPLY :
               decodeRouteReply(packet);
            break;

           case Packet::PACKETTYPE_EXPANDITEMREPLY :
               decodeExpandItemReply(packet);
            break;

            case Packet::PACKETTYPE_LOADMAPREPLY :
               decodeLoadMapReply(packet);
            break;

            case Packet::PACKETTYPE_DELETEMAPREPLY :
               decodeDeleteMapReply(packet);
            break;

            case Packet::PACKETTYPE_VANILLASEARCHREPLY :
               decodeVanillaSearchReply(packet);
            break;

            case Packet::PACKETTYPE_COVEREDIDSREPLYPACKET :
               decodeCoveredIDsReply(packet);
            break;

            case Packet::PACKETTYPE_TESTREPLY :
            {
               uint32 time = static_cast<TestReplyPacket*>(packet)
                  ->getTotalTime(TimeUtility::getCurrentMicroTime());
               m_meanResponseTime = (m_meanResponseTime* m_nbrResponses + time)
                  / (m_nbrResponses+1);
               m_nbrResponses++;
               if(time > 1000000)
                  m_failedResponses++;
               cout << "Test reply received!  ("
                    << time << "ms.)   nbr: "<< m_nbrResponses 
                    << ", failed: " << m_failedResponses << ", mean: "
                    << m_meanResponseTime << endl;
               cerr << packet->getPacketID() << ",";
               cerr << static_cast<TestReplyPacket*>(packet)
                  ->getModuleNbr()  <<  ",";
               cerr << static_cast<TestReplyPacket*>(packet)
                  ->getMapID() <<  ",";
               cerr << static_cast<TestReplyPacket*>(packet)
                  ->getProcessTime() << ",";
               cerr << static_cast<TestReplyPacket*>(packet)
                  ->getModuleTime() << ",";
               cerr << static_cast<TestReplyPacket*>(packet)
                  ->getReaderTime() << ",";
               cerr << static_cast<TestReplyPacket*>(packet)
                  ->getLeaderTime() << ",";
               cerr << static_cast<TestReplyPacket*>(packet)
                  ->getNetworkTime(TimeUtility::getCurrentMicroTime()) << endl;
               // static_cast<TestReplyPacket*>(packet)->dumpPacket();
            }
            break;
            case Packet::PACKETTYPE_USERSEARCHREPLY :
            break;
            case Packet::PACKETTYPE_OVERVIEWSEARCHREPLYPACKET :{
            	decodeOverviewSearchReplyPacket(packet);
            }

            case Packet::PACKETTYPE_USERGETDATAREPLY :
//               decodeUserGetDataReply(packet);
            break;
            case Packet::PACKETTYPE_USERADDREPLY :
//               decodeUserAddReply(packet);
            break;
            case Packet::PACKETTYPE_USERDELETEREPLY :
//               decodeUserDeleteReply(packet);
            break;
            case Packet::PACKETTYPE_USERCHANGEREPLY:
//               decodeUserChangeReply(packet);
            break;
            case Packet::PACKETTYPE_USERCHECKPASSWORDREPLY :
               decodeCheckUserPasswordReply( packet );
            break;
            case Packet::PACKETTYPE_USERFINDREPLY :
//               decodeUserFindReply(packet);
            break;
            case Packet::PACKETTYPE_USERSESSIONCLEANUPREPLY  :
               decodeSessionCleanUpReply( packet );
               break;
            case Packet::PACKETTYPE_GETUSERNAVDESTINATIONREPLYPACKET  :
               decodeGetUserNavDestinationReplyPacket( packet );
               break;  
            case Packet::PACKETTYPE_ADDUSERNAVDESTINATIONREPLYPACKET  :
               decodeAddUserNavDestinationReplyPacket( packet );
               break;

            case Packet::PACKETTYPE_ALLMAPREPLY :
               decodeAllMapReply(packet);
            break;
            case Packet::PACKETTYPE_COORDINATEREPLY :
               decodeCoordinateReply(packet);
            break;
            case Packet::PACKETTYPE_SMSREPLY:
               decodeSMSReply(packet);
               break;
            case Packet::PACKETTYPE_SENDEMAILREPLY:
               decodeEmailReply(packet);
               break;
            case Packet::PACKETTYPE_ADDDISTURBANCEREPLY:
               decodeAddDisturbanceReply(packet);
               cerr << "Add reply packet received! "<< endl;
               break;               
            case Packet::PACKETTYPE_REMOVEDISTURBANCEREPLY:
                decodeRemoveDisturbanceReply(packet);
                cerr << "Remove reply packet received! "<< endl;
               break;               
            case Packet::PACKETTYPE_DISTURBANCESUBSCRIPTIONREPLY:
               decodeDisturbanceSubscriptionReply(packet);
               cerr << "Subscription reply packet received! "<< endl;
               break;
            case Packet::PACKETTYPE_UPDATETRAFFICCOSTREQUEST:
               decodeUpdateTrafficCostRequest(packet);
               cerr << "Update traffic cost request received! "<< endl;
               break;
            case Packet::PACKETTYPE_EDGENODESREPLY:
               decodeEdgeNodesReply(packet);
               break;
            case Packet::PACKETTYPE_ACKNOWLEDGE:
            {
               if(static_cast<AcknowledgeRequestReplyPacket*>(packet)
                  ->getStatus() == StringTable::OK)
                  cout << "Ack received! "<< endl;
               else
                  cout << "Task refused! "<< endl;
            }
            break;
            case Packet::PACKETTYPE_ROUTEEXPANDITEMREPLY:
               decodeRouteExpandItemPacket(packet);
               break;
            default:
               cerr << "Unknown packet received! (type = "
                    << (int)packet->getSubType() << ")" << endl;
               break;
         }
      }
   }
}

void MTReceiver::decodeCoveredIDsReply(Packet* packet)
{
   CoveredIDsReplyPacket* inpacket = 
      static_cast<CoveredIDsReplyPacket*>(packet);
   if (inpacket != NULL) {
      uint32 n = inpacket->getNumberIDs();
      char* tmpStr = new char[n*16];
      char* ansStr = new char[32];
      sprintf(tmpStr, "CoveredIDsReplyPacket, %d answer(s)\n", n); 
      for (uint32 i=0; i<n; i++) {
         sprintf(ansStr, "   %d\n",   
                 inpacket->getID(i));
         strcat(tmpStr, ansStr);
      }
      cout << tmpStr;
      DEBUG8(cout << "allocated " << n*16 << " bytes, strlen(tmpStr)=" 
                  << strlen(tmpStr) << endl);
      log->putReply(packet->getPacketID(), tmpStr);
   }
} // MTReceiver:decodeCoveredIDsReply(...)

void MTReceiver::decodeAllMapReply(Packet* packet)
{
   AllMapReplyPacket* inPacket = static_cast<AllMapReplyPacket*>(packet);
   if ( inPacket != NULL ) {
      char* tmpStr = new char[1000];
      //if ( inPacket->getStatus() ) { // Should be StringTable::OK here
         sprintf(tmpStr, "AllMapReplyPacket\n"
                 "   # maps: %d\n",
                 inPacket->getNbrMaps());
         char* fooStr = new char[64];
         for (uint32 i=0; i<inPacket->getNbrMaps(); i++) {
            sprintf(fooStr, "%d ", inPacket->getMapID(i));
            strcat(tmpStr, fooStr);
         }
         delete [] fooStr;
      /*} else {
         sprintf(tmpStr, "AllMapReplyPacket\n  Status not OK\n");
      }*/
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr);
   } else {
      cout << "NULL" << endl;
   }
}

void MTReceiver::decodeCoordinateReply(Packet* packet)
{
   CoordinateReplyPacket* inPacket =
      static_cast<CoordinateReplyPacket *>(packet);
   if ( inPacket != NULL ) {
      char* tmpStr = new char[1024];
      if ( inPacket->getStatus() ) { // Should be StringTable::OK here
         const int maxLen = 128;
         char cou[maxLen];
         char mun[maxLen];
         char bua[maxLen];
         char dis[maxLen];
         char nam[maxLen];
         inPacket->getStrings(cou, mun, bua, dis, nam, maxLen);
         DEBUG8(cerr << "   Read from packet: " << cou << ", " << mun 
                     << ", " << bua << ", " << dis << ", " << nam << endl);
                                                                        
         sprintf(tmpStr, "CoordinateReplyPacket\n"
                 "   MapID: %d ItemID: %d Distance: %d Offset: %d "
                 "posAngle: %d negAngle: %d names:%s,%s,%s,%s,%s\n",
                 inPacket->getMapID(), inPacket->getItemID(), 
                 inPacket->getDistance(),
                 inPacket->getOffset(), inPacket->getPositiveAngle(),
                 inPacket->getNegativeAngle(),
                 cou, mun, bua, dis, nam);
      } else {
         sprintf(tmpStr, "CoordinateReplyPacket\n  Status not OK\n");
      }
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr);
   } else {
      cout << "NULL" << endl;
   }
}

void MTReceiver::decodeSMSReply(Packet* packet)
{
   SMSSendReplyPacket* inPacket = (SMSSendReplyPacket*)packet;
      if ( inPacket != NULL ) {
      char* tmpStr = new char[100];
      if ( inPacket->getStatus() == StringTable::OK ) { 
         sprintf(tmpStr, "SMSSendReply\n  Status OK\n");
      } else {
         sprintf(tmpStr, "SMSSendReplyPacket\n  Status %s\n", 
                 StringTable::getString((StringTable::stringCode)
                   inPacket->getStatus(),
                   StringTable::ENGLISH));
      }
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr);
   } else {
      cout << "NULL" << endl;
   }
}

void MTReceiver::decodeEmailReply(Packet* packet)
{
   SendEmailReplyPacket* inPacket = (SendEmailReplyPacket*) packet;
   if ( inPacket != NULL ) {
      char* tmpStr = new char[100];
      if ( inPacket->getStatus() == StringTable::OK ) { 
         sprintf(tmpStr, "SendEmailReplyPacket\n  Status OK\n");
      } else {
         sprintf(tmpStr, "SendEmailReplyPacket\n  Status not OK\n");
      }
      
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr);
   } else {
      cout << "MTReceiver::decodeEmailRepl: NULL" << endl;
   }
}

void MTReceiver::decodeTestReply(Packet* packet)
{
   TestReplyPacket* inpacket = static_cast<TestReplyPacket*>(packet);
   if (inpacket != NULL) {
      //char* tmpStr = new char[42];
      //sprintf(tmpStr, "TestReplyPacket\n");
      //cout << tmpStr;
      //log->putReply(packet->getPacketID(), tmpStr);
   }
} // MTReceiver:decodeTestReply(...)


void MTReceiver::decodeLoadMapReply(Packet* packet)
{
   LoadMapReplyPacket* inpacket = static_cast<LoadMapReplyPacket*>(packet);
   if (inpacket != NULL) {
      char* tmpStr = new char[256];
      StringTable::stringCode status = 
         StringTable::stringCode(inpacket->getStatus());
      if (status == StringTable::OK) {
         sprintf(tmpStr, "LoadMapReplyPacket\n   Map loaded ok\n");
      } else {
         sprintf(tmpStr, "LoadMapReplyPacket\n"
                         "   Failed to load map\n"
                         "   Message: \"%s\" (%d)\n",
                 StringTable::getString(status, StringTable::ENGLISH),
                 uint32(status));
      }
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr);
   }
} // MTReceiver:decodeLoadMapReply(...)

void MTReceiver::decodeDeleteMapReply(Packet* packet)
{
   DeleteMapReplyPacket* inpacket = static_cast<DeleteMapReplyPacket*>(packet);
   if (inpacket != NULL) {
      char* tmpStr = new char[256];
      StringTable::stringCode status = 
         StringTable::stringCode(inpacket->getStatus());
      if (status == StringTable::OK) {
         sprintf(tmpStr, "DeleteMapReplyPacket\n   Map deleted ok\n");
      } else {
         sprintf(tmpStr, "DeleteMapReplyPacket\n"
                         "   Failed to delete map\n"
                         "   Message: \"%s\" (%d)\n",
                 StringTable::getString(status, StringTable::ENGLISH),
                 uint32(status));
      }
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr);
   }
} // MTReceiver:decodeDeleteMapReply(...)


void MTReceiver::decodeMapReply(Packet* packet)
{
   MapReplyPacket* inpacket = static_cast<MapReplyPacket*>(packet);
   if (inpacket != NULL) {
      char* tmpStr = new char[128];
      sprintf(tmpStr, "MapReplyPacket:\n"
                      "   replyIP = %u\n"
                      "   replyPort = %u\n",
                      inpacket->getReplyIP(),
                      inpacket->getReplyPort());
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr);
      switch (inpacket->getRequestID()) {
         case MapRequestPacket::MAPREQUEST_SEARCH :
            getSearchMap(inpacket->getReplyIP(), inpacket->getReplyPort());
         break;
         case MapRequestPacket::MAPREQUEST_ROUTE :
            getRouteMap(inpacket->getReplyIP(), inpacket->getReplyPort());
         break;
         case MapRequestPacket::MAPREQUEST_GFX :
            getGfxMap(inpacket->getReplyIP(), inpacket->getReplyPort());
         break;
         case MapRequestPacket::MAPREQUEST_STRINGTABLE :
            getStringTableMap(inpacket->getReplyIP(),
                              inpacket->getReplyPort());
         break;
         default:
            cerr << "Unknown mapType!" << endl;
         break;
      }
   }
} // MTReceiver::sendMapRequest(...)


void 
MTReceiver::decodeCoordinateOnItemReply(Packet* packet)
{
   CoordinateOnItemReplyPacket* inpacket = 
      static_cast<CoordinateOnItemReplyPacket*>(packet);


   if ( (inpacket != NULL) &&
        (inpacket->getStatus() == StringTable::OK) ) {
      // Register the answer to the log
      char* tmpStr = new char[64];
      sprintf(tmpStr, "CoordinateOnItemReplyPacket\n");
      log->putReply(packet->getPacketID(), tmpStr);

      // Loop over all the items in the replyPacket
      uint32 nbrItems = inpacket->getNbrItems();
      cout << "CoordinateOnItemReplyPacket:" << endl;
      uint32 itemID = MAX_UINT32;
      int32 lat = GfxConstants::IMPOSSIBLE;
      int32 lon = lat;
      for (uint32 i=0; i<nbrItems; i++) {
         MC2BoundingBox bbox;
         inpacket->getLatLongAndBB(i, itemID, lat, lon, bbox);
         cout << "   " << i << " ID = " << itemID 
              << " (" << lat << ", " << lon << ')'
              << " [(" << bbox.getMaxLat() << ','
              << bbox.getMinLon() << "),(" << bbox.getMinLat()
              << ',' << bbox.getMaxLon() << ")]" 
              << endl;
      }
   } else {
      char* tmpStr = new char[64];
      sprintf(tmpStr, "ExpandItemReplyPacket = \n   Not Ok!\n");
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr); 
      // the tmpStr should not be deleted since it is handled to the MTLogLink
   }
}

void 
MTReceiver::decodeExpandItemReply(Packet* packet)
{
   ExpandItemReplyPacket* inpacket = 
         static_cast<ExpandItemReplyPacket*>(packet);
   cerr << "decodeExpandItemReplyPacket, status = " 
        << StringTable::getString(
               (StringTable::stringCode) inpacket->getStatus(), 
               StringTable::ENGLISH ) << endl;
   if ( (inpacket != NULL) &&
        (inpacket->getStatus() == StringTable::OK) ) {
      // Register the answer to the log
      char* tmpStr = new char[64];
      sprintf(tmpStr, "ExpandItemReplyPacket\n");
      log->putReply(packet->getPacketID(), tmpStr);
      // the tmpStr should not be deleted since it is handled to the MTLogLink

      // Loop over all the items in the replyPacket
      uint32 nbrExpandedIDs = inpacket->getNbrItems();
      cout << "ExpandItemReplyPacket, nbrItems = " << nbrExpandedIDs << endl;
      for (uint32 i=0; i<nbrExpandedIDs; i++) {
         cout << "   " << i << ". ID = " << inpacket->getItem(i) << endl;
      }
   } else {
      char* tmpStr = new char[64];
      sprintf(tmpStr, "ExpandItemReplyPacket = \n   Not Ok!\n");
      cout << tmpStr;
      log->putReply(packet->getPacketID(), tmpStr); 
      // the tmpStr should not be deleted since it is handled to the MTLogLink
   }
} // MTReceiver:sendItemRequest(...)


void MTReceiver::decodeRouteReply(Packet* packet)
{
   char* tmpStr = new char[65536];
   if (packet != NULL) {
      RouteReplyPacket* reply = (RouteReplyPacket*) packet;
      uint16 param;
      uint32 status;
      uint32 mapID;
      uint32 itemID;
      sprintf(tmpStr, "RouteReplyPacket =\n   PacketLength = %d\n",
              packet->getLength());
      uint32 nbr = reply->getNbrItems();
      sprintf(tmpStr+strlen(tmpStr), "   numberOfNodes = %u\n", nbr);
      status = reply->getStatus();
      sprintf(tmpStr+strlen(tmpStr), "   statusCode = %u = %s\n", 
              status,
              StringTable::getString(StringTable::stringCode(status), 
                                     StringTable::ENGLISH) );
      if ( (nbr > 0) && (status == StringTable::OK) ) {
         param = reply->getStartOffset();
         sprintf(tmpStr+strlen(tmpStr), "   StartOffset = %u\n", param);
         param = reply->getEndOffset();
         sprintf(tmpStr+strlen(tmpStr), "   EndOffset = %u\n", param);
         sprintf(tmpStr+strlen(tmpStr), "  MapID   Item   Dist    Time    "
                 "StandStill\n------------------------------------------\n");
         for (uint32 j = 0; j < nbr; j++) {
            reply->getRouteItem(j,
                                mapID,
                                itemID);
            sprintf(tmpStr+strlen(tmpStr), "%u\t%u\n", 
                    mapID, 
                    itemID); 

         }
      }
      cout << tmpStr << endl;
      log->putReply(packet->getPacketID(), tmpStr);
   }
} // MTReceiver::sendRouteRequest(...)


void MTReceiver::decodeVanillaSearchReply(Packet* packet)
{
   cout << "decodeVanillaSearchReply" << endl;
   VanillaSearchReplyPacket* inpacket =
                            static_cast<VanillaSearchReplyPacket*>(packet);
   if (inpacket != NULL) {
      int position; // initial value doesn't matter
      int nbrMatches;
      VanillaMatch *vm;
      vm = inpacket->getFirstMatch(position, nbrMatches);
      char* tmpStr = new char[27+8192*nbrMatches];
      sprintf(tmpStr, "Number of hits = %u\n", nbrMatches);
      while (vm!= NULL) {
         const char *tmp = vm->getName();         
         strcat(tmpStr, tmp);
         delete vm;
         vm = inpacket->getNextMatch(position);
      }
      cout << tmpStr << endl;
      log->putReply(packet->getPacketID(), tmpStr);
   }
} // MTReceiver:sendSearchRequest(...)


void 
MTReceiver::decodeCheckUserPasswordReply( Packet* packet ) {
   if ( packet != NULL ) {
      char* tmp = new char[1024];
      CheckUserPasswordReplyPacket* p = 
         static_cast<CheckUserPasswordReplyPacket*> ( packet );
      sprintf( tmp, "CheckUserPasswordReplyPacket\n"
               "   Status:     %s\n"
               "   UIN:        %d\n"
               "   SessionID:  %s\n"
               "   SessionKey: %s\n",
               StringTable::getString(
                  static_cast<StringTable::stringCode> ( p->getStatus() ), 
                  StringTable::ENGLISH ),
               p->getUIN(),
               p->getSessionID(),
               p->getSessionKey() );
      cout << tmp << endl;
      log->putReply( p->getPacketID(), tmp ); 
   }
}


void
MTReceiver::decodeSessionCleanUpReply( Packet* packet ) {
   if ( packet != NULL ) {
      char* tmp = new char[1024];
      SessionCleanUpReplyPacket* p = 
         static_cast<SessionCleanUpReplyPacket*> ( packet );
      sprintf( tmp, "SessionCleanUpReplyPacket\n"
               "   Status: %s\n",
               StringTable::getString(
                  static_cast<StringTable::stringCode> ( p->getStatus() ), 
                  StringTable::ENGLISH ) );               
      cout << tmp << endl;
      log->putReply( p->getPacketID(), tmp ); 
   }  
}


void 
MTReceiver::decodeGetUserNavDestinationReplyPacket( Packet* packet ) {
   GetUserNavDestinationReplyPacket* p = 
      static_cast<GetUserNavDestinationReplyPacket*> ( packet );
   char* tmp = new char[ 
      GetUserNavDestinationReplyPacket::MAX_DBUSERNAVIGATION_SIZE * 
      p->getNbrUserNavDestination() + 1024 ];
   char* pos = tmp;
   pos += sprintf( pos, "GetUserNavDestinationReplyPacket\n"
                   "   Status: %s\n"
                   "   NbrUserNavDestinations: %d\n"
                   "\n",
                   StringTable::getString(
                      static_cast<StringTable::stringCode> ( 
                         p->getStatus() ), 
                      StringTable::ENGLISH ),
                   p->getNbrUserNavDestination() );  
   int position = p->getFirstUserNavDestinationPos();
   for ( uint32 i = 0 ; i < p->getNbrUserNavDestination() ; i++ ) {
      DBUserNavDestination* nav = p->getUserNavDestination( position );
      // Add UserNavDestinations data
      pos += sprintf( pos, "   UserNavDestination:\n"
                      "      id: %d\n"
                      "      navigatorID: %d\n"
                      "      sent: %s\n"
                      "      createdDate: %d\n"
                      "      type: %d\n"
                      "      name: %s\n"
                      "      senderID: %d\n"
                      "      lat: %d\n"
                      "      lon: %d\n"
                      "\n",
                      nav->getID(),
                      nav->getNavigatorID(),
                      BP( nav->getSent() ),
                      nav->getCreationDate(),
                      nav->getMessageType(),
                      nav->getName(),
                      nav->getSenderID(),
                      nav->getLat(),
                      nav->getLon() );
      delete nav;
   }
   cout << tmp << endl;
   log->putReply( p->getPacketID(), tmp );   
}


void 
MTReceiver::decodeAddUserNavDestinationReplyPacket( Packet* packet ) {
   char* tmp = new char[4096];
   char* pos = tmp;
   AddUserNavDestinationReplyPacket* p = 
      static_cast<AddUserNavDestinationReplyPacket*> ( packet );
   pos += sprintf( pos, "AddUserNavDestinationReplyPacket\n"
                   "   Status: %s\n",
                   StringTable::getString(
                      static_cast<StringTable::stringCode> ( 
                         p->getStatus() ), 
                      StringTable::ENGLISH ) ); 
   cout << tmp << endl;
   log->putReply( p->getPacketID(), tmp );
}


void
MTReceiver::decodeAddDisturbanceReply( Packet* packet )
{
   /*
   if ( packet != NULL ) {
      char* tmp = new char[1024];
      AddDisturbanceReplyPacket* p = 
         static_cast<AddDisturbanceReplyPacket*> ( packet );
      sprintf( tmp, "AddDisturbanceReplyPacket\n"
               "   Status: %s\n"
               "   DistID: %d\n",
               StringTable::getString(
                  static_cast<StringTable::stringCode> ( p->getStatus() ), 
                  StringTable::ENGLISH ), p->getDisturbanceId());               
      cout << tmp << endl;
      log->putReply( p->getPacketID(), tmp ); 
   }    
   */
}


void
MTReceiver::decodeRemoveDisturbanceReply( Packet* packet )
{
   /*
   if ( packet != NULL ) {
      char* tmp = new char[1024];
      RemoveDisturbanceReplyPacket* p = 
         static_cast<RemoveDisturbanceReplyPacket*> ( packet );
      sprintf( tmp, "RemoveDisturbanceReplyPacket\n"
               "   Status: %s\n",
               StringTable::getString(
                  static_cast<StringTable::stringCode> ( p->getStatus() ), 
                  StringTable::ENGLISH ) );               
      cout << tmp << endl;
      log->putReply( p->getPacketID(), tmp ); 
   }    
   */
}

void
MTReceiver::decodeDisturbanceSubscriptionReply( Packet* packet )
{
   if ( packet != NULL ) {
      char* tmp = new char[1024];
      DisturbanceSubscriptionReplyPacket* p = 
         static_cast<DisturbanceSubscriptionReplyPacket*> ( packet );
      sprintf( tmp, "DisturbanceSubscriptionReplyPacket\n"
               "   Status: %s\n",
               StringTable::getString(
                  static_cast<StringTable::stringCode> ( p->getStatus() ), 
                  StringTable::ENGLISH ) );               
      cout << tmp << endl;
      log->putReply( p->getPacketID(), tmp ); 
   }
}

void
MTReceiver::decodeUpdateTrafficCostRequest(Packet* packet)
{
      uint32 sendToIP;
      uint16 sendToPort;
      if ( (packet != NULL) && 
           ( (sendToIP = packet->getOriginIP()) != 0) && 
           ( (sendToPort = packet->getOriginPort()) != 0))
      {
         
         UpdateTrafficCostReplyPacket* reply =
            new UpdateTrafficCostReplyPacket(static_cast<UpdateTrafficCostRequestPacket*>(packet));
         DatagramSender sock;
         if(! (sock.send(reply, sendToIP, sendToPort)) ) {
            cerr << "   MTReciever: reply error!!!" << endl;
         }
         else
            cerr << "   MTReciever: reply sent!!!" << endl;
      }
}

void MTReceiver::getSearchMap(uint32 ip, uint16 port)
{
   TCPSocket sock;
   uint32 i;
   uint32 j;
   uint32 nbr;
   uint32 nbr_j;
   uint32 len;
   uint32 mapID;
   DataBuffer sizeBuff(12);
   DataBuffer* dataBuffer;

   // Initialize connection
   sock.open();
   sock.connect(ip, port);
   sock.writeAll( (byte*)"Please give me a map", 20 );

   // Read string table
   sock.readExactly( sizeBuff.getBufferAddress(), 12 );
   mapID = sizeBuff.readNextLong();
   cout << "mapID = " << mapID << endl;
   len = sizeBuff.readNextLong();
   cout << "StringLength = " << len << endl;
   nbr = sizeBuff.readNextLong();
   cout << "NbrStrings = " << nbr << endl;
   dataBuffer = new DataBuffer(len);
   sock.readExactly( dataBuffer->getBufferAddress(), len );
   for (i = 0; i < nbr; i++)  {
      cout << "string[" << i << "] = "
           << dataBuffer->readNextString() << endl;
   }
   delete dataBuffer;

   // Read streets
   sizeBuff.reset();
   sock.readExactly( sizeBuff.getBufferAddress(), 8 );
   nbr = sizeBuff.readNextLong();
   cout << "nbrStreets = " << nbr << endl;
   len = sizeBuff.readNextLong();
   cout << "lengthOfStreets = " << len << endl;
   dataBuffer = new DataBuffer(len);
   sock.readExactly( dataBuffer->getBufferAddress(), len );
   for (i = 0; i < nbr; i++)  {
      cout << "street[" << i << "] =" << endl;
      cout << "   stringIndex = " << dataBuffer->readNextLong() << endl;
      cout << "   source = " << (int) dataBuffer->readNextByte() << endl;
      cout << "   timesSelected = " << dataBuffer->readNextShort() << endl;
      nbr_j = dataBuffer->readNextLong();
      cout << "   nbrStreetItems = " << nbr_j << endl;
      for (j = 0; j < nbr_j; j++) {
         cout << "   streetItem[" << j << "] =" << endl;
         cout << "      streetItemID = " << dataBuffer->readNextLong()
              << endl;
         cout << "      streetNumberType = "
              << (int)dataBuffer->readNextByte() << endl;
         cout << "      leftSideNumberStart = "
              << dataBuffer->readNextShort() << endl;
         cout << "      leftSideNumberEnd = "
              << dataBuffer->readNextShort() << endl;
         cout << "      rightSideNumberStart = "
              << dataBuffer->readNextShort() << endl;
         cout << "      rightSideNumberEnd = "
              << dataBuffer->readNextShort() << endl;
      }
   }
   delete dataBuffer;

   // Read companies
   sizeBuff.reset();
   sock.readExactly( sizeBuff.getBufferAddress(), 8 );
   nbr = sizeBuff.readNextLong();
   cout << "nbrCompanies = " << nbr << endl;
   len = sizeBuff.readNextLong();
   cout << "lengthOfCompanies = " << len << endl;
   dataBuffer = new DataBuffer(len);
   sock.readExactly( dataBuffer->getBufferAddress(), len );
   for (i = 0; i < nbr; i++)  {
      cout << "Company[" << i << "] =" << endl;
      cout << "   CompanyItemID = " << dataBuffer->readNextLong() << endl;
      cout << "   StringIndex = " << dataBuffer->readNextLong() << endl;
      cout << "   TimesSelected = " << dataBuffer->readNextShort() << endl;
      cout << "   source = " << (int) dataBuffer->readNextByte() << endl;
      cout << "   streetSegmentID = " << dataBuffer->readNextLong() << endl;
      cout << "   trueNumberOnStreet = " << dataBuffer->readNextLong()
           << endl;
      nbr_j = dataBuffer->readNextShort();
      cout << "   nbrCategories = " << nbr_j << endl;
      for (j = 0; j < nbr_j; j++) {
         cout << "   category[" << j << "] =" << dataBuffer->readNextLong()
              << endl;
      }
      cout << "   lengthOfPhoneNumbers = " << dataBuffer->readNextLong()
           << endl;
      nbr = dataBuffer->readNextLong();
      cout << "   nbrPhoneNumbers = " << nbr << endl;
      for (i = 0; i < nbr; i++)  {
         cout << "      phoneNumber[" << i << "] = "
              << dataBuffer->readNextString() << endl;
      }
   }
   delete dataBuffer;

   // Read categories
   sizeBuff.reset();
   sock.readExactly( sizeBuff.getBufferAddress(), 8 );
   nbr = sizeBuff.readNextLong();
   cout << "nbrCategories = " << nbr << endl;
   len = sizeBuff.readNextLong();
   cout << "lengthOfCategories = " << len << endl;
   dataBuffer = new DataBuffer(len);
   sock.readExactly( dataBuffer->getBufferAddress(), len );
   for (i = 0; i < nbr; i++)  {
      cout << "Category[" << i << "] =" << endl;
      cout << "   CategoryItemID = " << dataBuffer->readNextLong() << endl;
      cout << "   StringIndex = " << dataBuffer->readNextLong() << endl;
   }
   delete dataBuffer;
} // MTReceiver::getSearchMap(...)


void MTReceiver::getRouteMap(uint32 ip, uint16 port)
{
   TCPSocket sock;
   uint32 size,
          i,
          j,
          numConnections,
          *nodeVector;

   sock.open();
   sock.connect(ip, port);
   sock.writeAll( (byte*)"Please give me a map", 20 );
   sock.readExactly( (byte*)&size, 4 );
   size = ntohl(size);
   DataBuffer buff(size);
   sock.readExactly( (byte*)buff.getBufferAddress(), size );
   sock.close();

   size = buff.readNextLong();
   cout << "Number of nodes: " << size << endl;
   nodeVector = new uint32[size];

   for (i = 0; i < size; i++) {
      nodeVector[i] = buff.readNextLong();
   }
   for (i = 0; i < size; i++) {
      numConnections = buff.readNextLong();
      cout << "Node " << nodeVector[i] << " has " << numConnections
           << " connections:" << endl;
      for (j = 0; j < numConnections; j++){
         cout << "   connectionIndex    = " << buff.readNextLong() << endl;
         cout << "   costA              = " << buff.readNextLong() << endl;
         cout << "   costB              = " << buff.readNextLong() << endl;
         cout << "   costC              = " << buff.readNextLong() << endl;
         cout << "   costD              = " << buff.readNextLong() << endl;
         cout << "   vehicleRestriction = " << buff.readNextLong() << endl;
         cout << "   time               = " << buff.readNextLong() << endl;
         cout << "   dist               = " << buff.readNextLong() << endl;
         cout << "   standStillTime     = " << buff.readNextLong() << endl;
      }
   }
   delete [] nodeVector;
} // MTReceiver::getRouteMap(...)


void MTReceiver::getGfxMap(uint32 ip, uint16 port)
{
   TCPSocket sock;
   uint32 i;
   uint32 j;
   uint32 nbr;
   uint32 nbr_j = 0;
   uint32 len;
   DataBuffer sizeBuff(8);
   DataBuffer* dataBuffer;

   // Initialize connection
   sock.open();
   sock.connect(ip, port);
   sock.writeAll( (byte*)"Please give me a map", 20 );

   // Read the true-coordinates
   dataBuffer = new DataBuffer(32);
   uint32 kalle = sock.readExactly( dataBuffer->getBufferAddress(), 32 );
   cout << kalle << " bytes received on socket (32?)" << endl;
   for (int i=0; i<8; i++) {
      dataBuffer->readNextLong();
   }

   // Read item list
   sizeBuff.reset();
   sock.readExactly( sizeBuff.getBufferAddress(), 8 );
   nbr = sizeBuff.readNextLong();
   cout << "nbrItems = " << nbr << endl;
   len = sizeBuff.readNextLong();
   cout << "lengthOfItems = " << len << endl;
   dataBuffer = new DataBuffer(len);
   sock.readExactly( dataBuffer->getBufferAddress(), len );
   for (i = 0; i < nbr; i++) {
      cout << "item[" << i << "]:" << endl;

      cout << "   type = " << dataBuffer->readNextShort() << endl;
      byte nbrNames = dataBuffer->readNextByte();
      cout << "   nbrNames = " << (uint32) nbr_j << endl;
      byte nbrPolys = dataBuffer->readNextByte();
      cout << "   nbrPolygons = " << (uint32) nbrPolys << endl;

      cout << "   mapID = 0x" << hex << dataBuffer->readNextLong() << endl;
      cout << "   itemID = 0x" << dataBuffer->readNextLong() << dec << endl;
      for (j = 0; j < nbrNames; j++) {
         cout << "      name[" << j << "] = " << dataBuffer->readNextLong()
              << endl;
      }
      for (byte pol=0; pol<nbrPolys; pol++) {
         nbr_j = dataBuffer->readNextLong();
         cout << "   nbrCoordinates in polygon nbr " << (uint32) pol 
              << " = " << nbr_j << endl;
         for (j = 0; j < nbr_j; j++) {
            cout << "      coordinate[" << j << "] = ";
            cout << "(" << dataBuffer->readNextShort() << ", ";
            cout << dataBuffer->readNextShort() << ") ";
            cout << endl;
         }
      }
   }
   delete dataBuffer;
}

void MTReceiver::getStringTableMap(uint32 ip, uint16 port)
{
   TCPSocket sock;
   uint32 i;
   uint32 nbr;
   uint32 len;
   DataBuffer sizeBuff(8);
   DataBuffer* dataBuffer;

   // Initialize connection
   sock.open();
   sock.connect(ip, port);
   sock.writeAll( (byte*)"Please give me a map", 20 );

   // Read string data
   sock.readExactly( sizeBuff.getBufferAddress(), 8 );
   len = sizeBuff.readNextLong();
   cout << "StringLength = " << len << endl;
   nbr = sizeBuff.readNextLong();
   cout << "NbrStrings = " << nbr << endl;

   // Read string table
   dataBuffer = new DataBuffer(len);
   sock.readExactly( dataBuffer->getBufferAddress(), len );
   for (i = 0; i < nbr; i++)  {
      cout << "string[" << i << "] = "
           << dataBuffer->readNextString() << endl;
   }
   delete dataBuffer;
}


uint32 MTReceiver::getLastPacketTimeStamp() {
   return m_packetTimeStamp;
}


void MTReceiver::decodeOverviewSearchReplyPacket(Packet* packet)
{
	cout << "Received PACKETTYPE_OVERVIEWSEARCHREPLYPACKET" << endl;
   OverviewSearchReplyPacket* inpacket =
      static_cast<OverviewSearchReplyPacket*>(packet);
   if (inpacket != NULL) {
      const char* tmpStr = "OverviewReplyPacket";
      log->putReply(packet->getPacketID(), StringUtility::newStrDup(tmpStr)); 
//      delete tmpStr;
   } else {
      cerr << "Error, packet is not OverviewSearchReplyPacket" << endl;
   }
   cerr << "<done>" << endl;
} // void MTReceiver::decodeOverviewSearchReplyPacket(Packet* packet)

void MTReceiver::decodeEdgeNodesReply(Packet* packet)
{
   EdgeNodesReplyPacket* p = static_cast<EdgeNodesReplyPacket*>(packet);
   cerr << "Received " << p->getSubTypeAsString() << endl;
   OrigDestInfoList theList;
   p->getEdgeNodes(theList);
   cerr << "Status is "
        << StringTable::getString(StringTable::stringCode(p->getStatus()),
                                  StringTable::ENGLISH) << endl;
   cerr << "Nbr edgenodes  : " << theList.size() << endl;
   cerr << "Bordering maps : ";

   set<uint32> borderMaps;
   p->getBorderMaps(borderMaps);
   
   set<uint32>::iterator it(borderMaps.begin());
   while ( it != borderMaps.end() ) {
      cerr << *it++ << " ";
   }
   cerr << endl;
}

void
MTReceiver::decodeRouteExpandItemPacket(Packet* packet)
{ 
   RouteExpandItemReplyPacket* p =
      static_cast<RouteExpandItemReplyPacket*>(packet);

   if ( p->getStatus() == StringTable::OK ) {
      int nbrItems = p->getNbrItems();
      int pos = p->positionInit();
      for(int i=0; i < nbrItems; ++i ) {
         if ( i != 0 )
            cerr << "------------" << endl;
         cerr << "------------" << endl;
         cerr << "Item number " << i << endl;
         uint16 nbrPositions = 0;
         uint32 expandedItemID = MAX_UINT32;
         ItemTypes::itemType type = ItemTypes::nullItem;
         uint32* ssItemIDs = NULL;
         int32* lats = NULL;
         int32* lons = NULL;
         uint16* offsets = NULL;
         uint32* parentItemIDs = NULL;
         p->getItem(pos, nbrPositions, 
                    expandedItemID, type,
                    ssItemIDs, offsets, 
                    lats, lons, parentItemIDs);
         
         cerr << "expandedItemID = " << expandedItemID << endl;
         cerr << "type = " << int(type) << endl;;
         for (uint32 j=0; j<nbrPositions; j++) {
            // Print
            cerr << "Part number " << j << endl;
            cerr << "lat[" << j << "] = " << lats[j] << endl;
            cerr << "lon[" << j << "] = " << lons[j] << endl;
            cerr << "itemID[" << j << "] = " << hex << ssItemIDs[j]
                 << dec << endl;
            cerr << "offset[" << j << "] = " << offsets[j] << endl;         
         }
         
         // Clean up
         delete ssItemIDs;
         delete lats;
         delete lons;
         delete offsets;
         delete parentItemIDs;
      }
   } else {
      StringTable::stringCode status = StringTable::stringCode(p->getStatus());
      cerr << "Status was \"" << StringTable::getString(status,
                                                      StringTable::ENGLISH)
           << "\"" << endl;
   }
}
