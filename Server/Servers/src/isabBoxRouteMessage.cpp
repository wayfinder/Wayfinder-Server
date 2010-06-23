/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "isabBoxRouteMessage.h"
#include "isabRouteList.h"
#include "isabRouteElement.h"
#include "NavUtil.h"
#include "RouteRequest.h"
#include "ExpandedRoute.h"


isabBoxRouteReq::isabBoxRouteReq(const NavAddress& senderAddress, NavSession * session)
      : RouteReq(senderAddress, session)
{
   
}

bool
isabBoxRouteReq::convertFromBytes(const byte* buf,
                                  int bufSize)
{
   int pos = isabBoxNavMessageUtil::convertHeaderFromBytes(this, buf, bufSize);
   if ( pos == 0 ) /* Header decoding failed */
      return false; 
   
   /* Real packet decoding here */
   m_origLat = (int32)isabBoxNavMessageUtil::incReadLong(buf, pos);
   m_origLon = (int32)isabBoxNavMessageUtil::incReadLong(buf, pos);
   m_destLat = (int32)isabBoxNavMessageUtil::incReadLong(buf, pos);
   m_destLon = (int32)isabBoxNavMessageUtil::incReadLong(buf, pos);
   uint16 packetAngle = MAX_UINT16;
   m_speed = 0;
   if ( getProtoVer() >= 0x05 ) {
      if ( getProtoVer() >= 0x07 ) {
         m_language = StringTable::ENGLISH;
         m_timeToTrunk = isabBoxNavMessageUtil::incReadLong( buf,pos );
         packetAngle = isabBoxNavMessageUtil::incReadShort( buf, pos );
         m_speed = isabBoxNavMessageUtil::incReadShort( buf, pos );
         m_routeType = RouteReq::routeType( 
            isabBoxNavMessageUtil::incReadByte( buf,pos ) );
      } else {
         m_language = NavUtil::mc2LanguageCode( 
            isabBoxNavMessageUtil::incReadLong( buf,pos) );
         m_timeToTrunk = MAX_UINT32;
         m_routeType = RouteReq::full;
      }
      m_vehicle = NavUtil::mc2Vehicle( isabBoxNavMessageUtil::incReadByte(
                                          buf, pos ) );
      // MSB in routeCost is avoidTollroads
      uint8 routeCost = isabBoxNavMessageUtil::incReadByte( buf, pos );
      m_avoidTollRoads = (routeCost>>7) != 0;
      m_avoidHighway = ((routeCost>>6) & 0x01) != 0;
      // landmarks 
      m_landmarks = ((routeCost>>5) & 0x01) != 0;
      // abbreviation
      m_abbreviate = !(((routeCost>>4) & 0x01) != 0);
      m_routeCost = NavUtil::mc2RouteCost( (routeCost & 0xf) );
      if ( getProtoVer() >= 0x07 ) {
         // Route id
         m_routeID = isabBoxNavMessageUtil::incReadLong( buf,pos );
         // Route create time
         m_routeCreateTime = isabBoxNavMessageUtil::incReadLong( buf,pos );
      }
   } else {
      m_language = StringTable::ENGLISH;
      m_vehicle = ItemTypes::passengerCar;
      m_routeCost = RouteTypes::TIME;
   }
   mc2dbg2 << "m_language " << int(m_language) << endl;
   mc2dbg2 << "m_vehicle " << uint32(m_vehicle) << endl;
   mc2dbg2 << "m_routeCost " << int(m_routeCost) << endl;
   // Read out the start angle from packet (in 256, DEG correct it to 360 DEG)
   if ( getProtoVer() < 0x07 ) { // Is above if >= 0x07
      packetAngle = isabBoxNavMessageUtil::incReadByte(buf, pos);
   }
   if ( packetAngle > 255 ) {
      m_startAngle = MAX_UINT16;
   } else {
      m_startAngle = uint16(double(packetAngle) * 360.0 / 256.0);
   }

   mc2dbg2 << "isabBox Coord " << endl;
   mc2dbg2 << "m_origLat " << m_origLat << endl;
   mc2dbg2 << "m_origLon " << m_origLon << endl;
   mc2dbg2 << "m_destLat " << m_destLat << endl;
   mc2dbg2 << "m_destLon " << m_destLon << endl;
   mc2dbg2 << "m_timeToTrunk " <<  m_timeToTrunk << endl;
   mc2dbg2 << "m_routeType " << int(m_routeType) << endl;
   mc2dbg2 << "packetAngle (0-255)" << (uint32)packetAngle << endl;
   mc2dbg2 << "m_startAngle (deg)" << m_startAngle << endl;
   mc2dbg2 << "m_speed " << (int)m_speed << endl;
   mc2dbg2 << "m_avoidTollRoads " << m_avoidTollRoads << endl;

   /* Check if the origin coordinate is valid. */
   if (!isabBoxNavMessageUtil::validNavCoordinates(m_origLat, m_origLon)) {
      /* No, the coordinates are invalid. */
      /* This is a fatal error for Route request. */
      mc2log << error << "Coordinates not valid in route request." << endl;
      m_validCoordinates = false;
      m_origLat = MAX_INT32;
      m_origLon = MAX_INT32;
   } else {
      m_validCoordinates = true;
      /* Convert from navRad to MC2 unit */
      m_origLat = isabBoxNavMessageUtil::navRadToMC2(m_origLat);
      m_origLon = isabBoxNavMessageUtil::navRadToMC2(m_origLon);
   }

   if (m_destLat != MAX_INT32) {
      m_destLat = isabBoxNavMessageUtil::navRadToMC2(m_destLat);
      m_destLon = isabBoxNavMessageUtil::navRadToMC2(m_destLon);
   }

   mc2dbg2 << "mc2 Coord " << endl;
   mc2dbg2 << "m_origLat " << m_origLat << endl;
   mc2dbg2 << "m_origLon " << m_origLon << endl;
   mc2dbg2 << "m_destLat " << m_destLat << endl;
   mc2dbg2 << "m_destLon " << m_destLon << endl;
           
   
   return true;
}
                             
/*-------------------------------------------------------*/

isabBoxRouteReply::isabBoxRouteReply(const NavAddress& recipientAddress,
                                     PacketContainer* expansionAnswer,
                                     uint32 routeID, 
                                     uint32 routeCreateTime,
                                     RouteReq* req,
                                     NavSession* session,
                                     RouteRequest* rr,
                                     uint8 reqVer )
      : RouteReply( recipientAddress, expansionAnswer, routeID, 
                    routeCreateTime, req, session, rr )
{
   m_length = 0;
   m_irl = new IsabRouteList( m_protoVer, &m_routeList,
                              &m_routeList.getStringTable(),
                              m_session, reqVer );
}

isabBoxRouteReply::isabBoxRouteReply(const NavAddress& recipientAddress,
                                     PacketContainer* expansionAnswer,
                                     uint32 routeID, 
                                     uint32 routeCreateTime,
                                     int protoVer,
                                     NavSession * session,
                                     uint8 reqVer )
      : RouteReply( recipientAddress, expansionAnswer, routeID, 
                    routeCreateTime, protoVer, session, NULL )
{
   m_length = 0;
   m_irl = new IsabRouteList( m_protoVer, &m_routeList,
                              &m_routeList.getStringTable(),
                              m_session, reqVer );
}


isabBoxRouteReply::~isabBoxRouteReply() {
   delete m_irl;
}


bool
isabBoxRouteReply::convertToBytes(byte *buf,
                                  int bufSize)
{
   int pos = isabBoxTypeMessage::getHeaderSize( m_protoVer );

   NavStringTable& navStringTable = m_routeList.getStringTable();
   
   if ( !m_irl->isValid() ) {
      mc2log << warn << "Route is invalid!" << endl;
   }

   mc2dbg2 << "IRL SIZE = " <<  m_irl->getSize() << endl;
   mc2dbg2 << "IRL USED STRING SIZE = " <<  m_irl->getNbrStringsUsed() 
           << endl;

   // Number of used strings in NavStringTable   
   int stringSize = m_irl->getNbrStringsUsed();
   // Size of the route (may be truncated)
   int listSize   = m_irl->getSize();
   
   /* Write the status byte */
   if ( getStatus() == NAV_STATUS_OK && m_irl->isValid()) {
      if ( getProtoVer() < 0x05 ) { // Status not in header
         isabBoxNavMessageUtil::incWriteByte( buf, pos, getStatus() );
      }
      mc2dbg2 << "Route OK, Proto ver: " << int(m_protoVer) << endl;
      mc2dbg2 << "Total dist: " << m_routeList.getTotalDist() << endl;

      if ( m_irl->isTruncated() ) {
         mc2dbg2 << "Route is truncated !" << endl;
         mc2dbg2 << "Truncated dist: " << m_irl->getTruncatedDist() 
                << endl;
         mc2dbg2 << "Distance left after this part: " <<
            m_routeList.getTotalDist()-
            m_irl->getTruncatedDist() << endl;            
      }
      // This should not be nessesary as header should be even nbr bytes
      isabBoxNavMessageUtil::alignShort( buf, pos );

      if ( m_protoVer >= 0x06 ) {
         // Route id first after status
         isabBoxNavMessageUtil::incWriteLong( buf, pos, m_routeID );
         isabBoxNavMessageUtil::incWriteLong( 
            buf, pos, m_routeCreateTime );
         mc2dbg2 << "RouteID " << m_routeID << " createTime "
                << m_routeCreateTime << endl;
      }
      if ( m_protoVer >= 0x08 ) {
         // Route bbox first after routeID
         MC2BoundingBox bbox = MC2BoundingBox();
         if ( m_routeList.getRouteRequest()->getExpandedRoute() ) {
            bbox = m_routeList.getRouteRequest()->
               getExpandedRoute()->getRouteBoundingBox();
         }
         // Top Latitude 
         isabBoxNavMessageUtil::incWriteLong( 
            buf, pos, isabBoxNavMessageUtil::MC2ToNavRad( 
               bbox.getMaxLat() ) );
         // Left Longitude
         isabBoxNavMessageUtil::incWriteLong( 
            buf, pos, isabBoxNavMessageUtil::MC2ToNavRad(
               bbox.getMinLon() ) );
         // Bottom Latitude
         isabBoxNavMessageUtil::incWriteLong( 
            buf, pos, isabBoxNavMessageUtil::MC2ToNavRad(
               bbox.getMinLat() ) );
         // Right Longitude
         isabBoxNavMessageUtil::incWriteLong( 
            buf, pos, isabBoxNavMessageUtil::MC2ToNavRad(
              bbox.getMaxLon() ) );
      }

      if (m_protoVer > 0) {
         // not used in version == 0
         if ( m_irl->isTruncated() ) {
            // write the distance left to goal after this part of route.
            // so that we can show a totalDist even if it's only
            // part of a route
            isabBoxNavMessageUtil::incWriteLong(
               buf,
               pos,
               m_routeList.getTotalDist()-
               m_irl->getTruncatedDist()
               );
            if ( m_protoVer >= 0x05 ) {
               // dist2nextWPTFromTrunk
               isabBoxNavMessageUtil::incWriteLong(
                  buf, pos,
                  m_irl->getDist2nextWPTFromTrunk() );
            }
         } else {
            isabBoxNavMessageUtil::incWriteLong(buf, pos, 0 );
            if ( m_protoVer >= 0x05 ) {
               // dist2nextWPTFromTrunk
               isabBoxNavMessageUtil::incWriteLong( buf, pos, 0 );
            }
         }      
         if ( m_irl->isTruncated() ) {
            // The distance from end of truncated route when the
            // navigator will call
            uint32 phoneHomeDist = m_irl->getTruncatedDist() / 2;
            if ( phoneHomeDist > 2000 ) {
               phoneHomeDist = 2000;
            }
            isabBoxNavMessageUtil::incWriteLong( buf, pos, phoneHomeDist );
         } else {
            isabBoxNavMessageUtil::incWriteLong(buf, pos, 0);
         }
      }
      
      // write all the strings
      int charSize = 0;
      for(int i = 0; i <= stringSize; i++ ) {
         // calculate the size of all strings including '\0'
         // BUT UTF-8 STRINGS WILL BE CONVERTED BY incWriteString
         // IF PROTOVER <= 0xa !!!
         // WHICH IT WILL BE IF IT ISN'T THE JAVA CLIENT
         charSize += strlen(navStringTable[i]) + 1;
      }
      
      isabBoxNavMessageUtil::alignShort( buf, charSize ); // Will align 
                                                         // the route later
      int charSizePos = pos;
      isabBoxNavMessageUtil::incWriteShort(buf, pos, charSize);      
      mc2dbg2 << "stringSize: " << stringSize+1 << endl;

      for(int i = 0; i <= stringSize; i++ ) {
         if ( m_protoVer <= 0xa  ) {
            // Use latin-1
            isabBoxNavMessageUtil::incWriteString(buf,
                                                  pos,
                                                  navStringTable[i]);
         } else {
            // Use UTF-8
            // Can only happen with java clients.
            isabBoxNavMessageUtil::incWriteStringUTF8( buf,
                                                       pos,
                                                       navStringTable[i] );
         }
         mc2dbg2 << "name:\t" << navStringTable[i] << endl;
      }

      // Write real string size in buffer
      isabBoxNavMessageUtil::incWriteShort( buf, charSizePos, 
                                            pos - (charSizePos+2) );

      // Align route. The navigator needs 2 bytes alignment
      isabBoxNavMessageUtil::alignShort( buf, pos );
      
      // Get the size of the routelist
      
      //write route element data
      for (int i = 0; i < listSize; i++ ) {
         const IsabRouteElement* ire = m_irl->isabRouteElementAt(i);
         pos = ire->write(buf, pos);
      }

//      HEXDUMP(cerr, buf, pos,"  ");
      
      
   } else {
      mc2log << warn << "Route NOTOK !!" << endl;
      if ( getProtoVer() < 0x05 ) { // Status not in header
         isabBoxNavMessageUtil::incWriteByte( buf, pos, getStatus() );
      }
      if ( m_protoVer >= 0x06 ) {
         // Route id first after status
         isabBoxNavMessageUtil::incWriteLong( buf, pos, 0 );
         isabBoxNavMessageUtil::incWriteLong( buf, pos, 0 );
      }
      /* Write the number of route elements == 0 */
      isabBoxNavMessageUtil::incWriteShort(buf, pos, 0);
   }
   setLength(pos);
   mc2dbg2 << "Total length for ROUTE_REPLY " << pos << endl;
   isabBoxNavMessageUtil::convertHeaderToBytes(this, buf, bufSize);


   // Check if we should create binary packet for addtional route data.
   if ( static_cast<isabBoxSession*>( m_session )
        ->getDownloadAdditionalRouteData() ) 
   {
      // The additional route data is requested.
      if ( m_irl->hasAdditionalRouteData() ) {
         mc2dbg2 << "Will create binary data from rest of route."
                << endl;
         // And the data is there (i.e. the route did not fit in one packet.
         int safetyMargin = 50;
         bufSize = m_session->getMaxBufferLength()-safetyMargin;
         if (bufSize <= 0) {
            bufSize = 50;
         }
         byte *buf = new byte[bufSize+safetyMargin];

         int headerSize = 4;
         pos = 0;
         listSize   = m_irl->getSizeAdditional();

         int numBufs = ((listSize*isabBoxNavMessageUtil::navigatorStructSize)
               / bufSize) + 1;
         int thisBufNum = 0;

         mc2dbg2 << "Will create " << numBufs << " buffers of size "
                << bufSize << endl;
         mc2dbg2 << "Listsize: " << listSize << " datasize: "
                << (listSize*isabBoxNavMessageUtil::navigatorStructSize)
                << " data/bufsize: "
                << ((listSize*isabBoxNavMessageUtil::navigatorStructSize) / bufSize)
                << endl;

         isabBoxNavMessageUtil::incWriteLong(buf, pos,
               isabBoxNavMessageUtil::additionalRouteData);
         isabBoxNavMessageUtil::incWriteShort(buf, pos, thisBufNum);
         isabBoxNavMessageUtil::incWriteShort(buf, pos, numBufs);

         //write route element data
         for (int i = 0; i < listSize; i++ ) {
            const IsabRouteElement* ire = 
               m_irl->isabAdditionalRouteElementAt( i );
            pos = ire->write(buf, pos);

            if (pos >= bufSize) {
               // Large enough, don't want it any bigger.
               mc2dbg2 << "Buffer " << thisBufNum << " is "
                      << pos << " bytes" << endl;
               // Add this buffer to the binary data and
               // allocate a new buffer.
               m_session->addData(buf, pos);
               pos = 0;
               buf = new byte[bufSize+safetyMargin];
               thisBufNum++;
               isabBoxNavMessageUtil::incWriteLong(buf, pos,
                     isabBoxNavMessageUtil::additionalRouteData);
               isabBoxNavMessageUtil::incWriteShort(buf, pos, thisBufNum);
               isabBoxNavMessageUtil::incWriteShort(buf, pos, numBufs);
            }
         }
         if (pos > headerSize) {
            // Only add the data if there is any.
            m_session->addData(buf, pos);
            mc2dbg2 << "Last buffer " << thisBufNum << " is "
                   << pos << " bytes" << endl;
         } else {
            // No elements in this buffer, throw it away.
            delete[] buf;
         }
      }
      mc2dbg2 << "Additional data done." << endl;
   }
   mc2dbg2 << "All data done." << endl;

   return true;
}

