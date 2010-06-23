/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSFerryItem.h"
#include "GMSMidMifHandler.h"
#include "MidMifData.h"
#include "Utility.h"

GMSFerryItem::GMSFerryItem(bool createNodes)
   :  OldFerryItem(),
      GMSItem(GMSItem::GDF)
{
   initMembers(MAX_UINT32, createNodes);
}

GMSFerryItem::GMSFerryItem(uint32 id, bool createNodes) 
   :  OldFerryItem(id),
      GMSItem(GMSItem::GDF)
{
   initMembers(id, createNodes);
}

GMSFerryItem::GMSFerryItem(GMSItem::map_t mapType, bool createNodes) 
   :  OldFerryItem(),
      GMSItem(mapType)
{
   initMembers(MAX_UINT32, createNodes);
}

GMSFerryItem::GMSFerryItem(GMSFerryItem* ferry)
   :  OldFerryItem(),
      GMSItem(GMSItem::GDF)
{
   initMembers(MAX_UINT32, false);
   
   m_node0 = NULL;
   m_node1 = NULL;
   copyAttributes(ferry);
}

void
GMSFerryItem::initMembers(uint32 id, bool createNodes)
{
   m_roadClass = 0;
   m_ferryType = 0;
   init(ItemTypes::ferryItem);
   if (createNodes) {
      if (id != MAX_UINT32) {
         m_node0 = new GMSNode(id & 0x7FFFFFFF);
         m_node1 = new GMSNode(id | 0x80000000);
      } else {
         m_node0 = new GMSNode();
         m_node1 = new GMSNode();
      }
      // Set speedlimit to 20km/h
      m_node0->setSpeedLimit(20);
      m_node1->setSpeedLimit(20);
   }
   
}

GMSFerryItem::~GMSFerryItem()
{
}

bool
GMSFerryItem::createFromMidMif(ifstream& midFile, bool readRestOfLine)
{
   const uint32 maxLineLength = GMSMidMifHandler::maxMidLineLength;
   
   //create empty nodes
   if (m_node0 == NULL)
      m_node0 = new GMSNode();
   if (m_node1 == NULL)
      m_node1 = new GMSNode();
   
   char inbuffer[maxLineLength];
   char* tmpchr;

   //roadClass
   //read past first ','
   midFile.getline(inbuffer,maxLineLength, ',');
   //read the roadclass value
   midFile.getline(inbuffer,maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int tmpRoadClass = strtol(inbuffer, &tmpchr, 10);
      mc2dbg8 << "  RoadClass = " << tmpRoadClass << endl;
      m_roadClass = (byte) tmpRoadClass;
   }
   
   //posspeed
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int32 posSpeed = strtol(inbuffer, &tmpchr, 10);
      if (posSpeed < 0) {
         posSpeed = -1;
         ItemTypes::entryrestriction_t restriction = ItemTypes::noWay;
         getNode(0)->setEntryRestrictions(restriction);
         mc2dbg8 << " setting entryrestrictions to NoWay due to "
                     << "pos-speed " << posSpeed << endl;
      }
      mc2dbg8 << "  Posspeed = " << posSpeed << endl;
      getNode(0)->setSpeedLimit( (uint8) posSpeed);
      // Speed always 20 for ferry!
      getNode(0)->setSpeedLimit( 20 );
   }
   
   //negspeed
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int32 negSpeed = strtol(inbuffer, &tmpchr, 10);
      if (negSpeed < 0) {
         negSpeed = -1;
         ItemTypes::entryrestriction_t restriction = ItemTypes::noWay;
         getNode(1)->setEntryRestrictions(restriction);
         mc2dbg8 << " setting entryrestrictions to NoWay due to "
                 << "neg-speed " << negSpeed << endl;
      }
      mc2dbg8 << "  Negspeed = " << negSpeed << endl;
      getNode(1)->setSpeedLimit( (uint8) negSpeed);
      // Speed always 20 for ferry!
      getNode(1)->setSpeedLimit( 20 );
   }

   //pos entryrestrictions (entering from node 0)
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int tmpPosRes = strtol(inbuffer, &tmpchr, 10);
      if ( (tmpPosRes >= 0) && (tmpPosRes < 4)) {
         ItemTypes::entryrestriction_t restriction = 
            ItemTypes::entryrestriction_t (tmpPosRes);
         getNode(0)->setEntryRestrictions(restriction);
         mc2dbg8 << " pos restriction " << tmpPosRes << endl;
      }
   }

   //neg entryrestrictions (entering from node 1)
   midFile.getline(inbuffer, maxLineLength, ',');
   if (strlen(inbuffer) > 0) {
      int tmpNegRes = strtol(inbuffer, &tmpchr, 10);
      if ( (tmpNegRes >= 0) && (tmpNegRes < 4)) {
         ItemTypes::entryrestriction_t restriction = 
            ItemTypes::entryrestriction_t (tmpNegRes);
         getNode(1)->setEntryRestrictions(restriction);
         mc2dbg8 << " neg restriction " << tmpNegRes << endl;
      }
   }

   //level node 0
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpLevel = strtol(inbuffer, &tmpchr, 10);
      getNode(0)->setLevel(tmpLevel);
      mc2dbg8 << " node level 0 " << tmpLevel << endl;
   }

   //level node 1
   midFile.getline(inbuffer, maxLineLength, ',');
   if( strlen(inbuffer) > 0 ) {
      int tmpLevel = strtol(inbuffer, &tmpchr, 10);
      getNode(1)->setLevel(tmpLevel);
      mc2dbg8 << " node level 1 " << tmpLevel << endl;
   }

   //roadToll
   midFile.getline(inbuffer, maxLineLength, '"');
   midFile.getline(inbuffer, maxLineLength, '"');
   mc2dbg8 << " road toll " << inbuffer << endl;
   if (Utility::getBoolFromString(inbuffer)) {
      getNode(0)->setRoadToll(true);
      getNode(1)->setRoadToll(true);
   }
 
   // Handle ferryType, default = operatedByShip
   setFerryType(ItemTypes::operatedByShip);
   
   // Check if we have any extra attributes to read,
   // don't count any map ssi coordinates
   uint32 nbrExtraAttributes = 
         GMSMidMifHandler::getNumberExtraAttributes( midFile, !readRestOfLine);

   // Read any extra item attributes
   // Extra: ferry type
   if ( nbrExtraAttributes > 0 ) {
      // read the "'" char after roadToll
      midFile.getline(inbuffer, maxLineLength, ',');
      // If this is the last attribute in the mid row there is no more 
      // "," char to read
      if ( (nbrExtraAttributes == 1) && readRestOfLine ) {
         midFile.getline(inbuffer, maxLineLength);
         readRestOfLine = false;
      } else {
         midFile.getline(inbuffer, maxLineLength, ',');
      }
      if ( strlen(inbuffer) > 0 ) {
         uint32 tmpFerryType = strtoul(inbuffer, &tmpchr, 10);
         setFerryType( ItemTypes::ferryType(tmpFerryType) );
         mc2dbg8 << " ferry type " << tmpFerryType << endl;
      }
      nbrExtraAttributes--;
   }
   // Extra: node 0 border node
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      if (Utility::getBoolFromString(inbuffer)) {
         mc2dbg8 << " node 0 border node: " << inbuffer << endl;
         getNode(0)->setJunctionType( ItemTypes::borderCrossing );
      }
      nbrExtraAttributes--;
   }
   // Extra: node 1 border node
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      if (Utility::getBoolFromString(inbuffer)) {
         mc2dbg8 << " node 1 border node: " << inbuffer << endl;
         getNode(1)->setJunctionType( ItemTypes::borderCrossing );
      }
      nbrExtraAttributes--;
   }
   
   /*
   // Template for next item attribute
   if ( nbrExtraAttributes > 0 ) {
      midFile.getline(inbuffer, maxLineLength, '"');
      midFile.getline(inbuffer, maxLineLength, '"');
      nbrExtraAttributes--;
   }*/

   if (nbrExtraAttributes != 0) {
      mc2log << error << here << "Unused extra attributes: "
             << nbrExtraAttributes << endl;
   }

   // Skip the rest of the mid line, nothing there
   if (readRestOfLine)
      midFile.getline(inbuffer, maxLineLength);
   // else
   // Don't read the rest of the mid line, we have a map ssi coordinate 
   // to use for checking which map is the correct one
 
   return true;
}

void
GMSFerryItem::copyAttributes(GMSFerryItem* ferry)
{
   if (m_node0 == NULL)
      m_node0 = new GMSNode();
   if (m_node1 == NULL)
      m_node1 = new GMSNode();

   m_roadClass = ferry->getRoadClass();
   m_ferryType = ferry->getFerryType();

   //node0
   getNode(0)->setSpeedLimit(ferry->getNode(0)->getSpeedLimit());
   getNode(0)->setEntryRestrictions(ferry->getNode(0)->getEntryRestrictions());
   getNode(0)->setMaximumHeight(ferry->getNode(0)->getMaximumHeight());
   getNode(0)->setMaximumWeight(ferry->getNode(0)->getMaximumWeight());
   getNode(0)->setLevel(ferry->getNode(0)->getLevel());
   getNode(0)->setMajorRoad(ferry->getNode(0)->isMajorRoad());
   getNode(0)->setRoadToll(ferry->getNode(0)->hasRoadToll());

   //node1
   getNode(1)->setSpeedLimit(ferry->getNode(1)->getSpeedLimit());
   getNode(1)->setEntryRestrictions(ferry->getNode(1)->getEntryRestrictions());
   getNode(1)->setMaximumHeight(ferry->getNode(1)->getMaximumHeight());
   getNode(1)->setMaximumWeight(ferry->getNode(1)->getMaximumWeight());
   getNode(1)->setLevel(ferry->getNode(1)->getLevel());
   getNode(1)->setMajorRoad(ferry->getNode(1)->isMajorRoad());
   getNode(1)->setRoadToll(ferry->getNode(1)->hasRoadToll());

}

