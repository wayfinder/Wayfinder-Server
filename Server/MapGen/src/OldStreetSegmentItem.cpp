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

#include "OldNode.h"
#include "OldStreetSegmentItem.h"

#include "MC2String.h"
#include "DataBuffer.h"

#include "Utility.h"

const byte OldStreetSegmentItem::WIDTH_MASK = 0x0f;

const byte OldStreetSegmentItem::ROUNDABOUTISH_MASK = 0x10;

const byte OldStreetSegmentItem::STREETNUMBERTYPE_MASK = 0x07;

const byte OldStreetSegmentItem::ROUNDABOUT_MASK = 0x08;

const byte OldStreetSegmentItem::RAMP_MASK = 0x10;

const byte OldStreetSegmentItem::DIVIDED_MASK = 0x20;

const byte OldStreetSegmentItem::MULTI_DIGITISED_MASK = 0x40;

const byte OldStreetSegmentItem::CONTROLLED_ACCESS_MASK = 0x80;

OldStreetSegmentItem::OldStreetSegmentItem(uint32 id)
                  : OldRouteableItem(ItemTypes::streetSegmentItem, id) 
{
   DEBUG_DB(mc2dbg << "OldStreetSegmentItem created" << endl);

   // Set the attributes for this OldItem
   m_condition = 0;
   m_roadClass = 0;
   m_width = 0;
   m_streetNumberType = ItemTypes::noStreetNumbers;
   m_roundabout = false;
   m_roundaboutish = false;
   m_ramp = false;
   m_divided = false;
   m_multiDigitised = false;
   m_controlledAccess = false;
   
   m_leftsideNumberStart = 0;
   m_leftsideNumberEnd = 0;

   m_rightsideNumberStart = 0;
   m_rightsideNumberEnd = 0;
}

uint32 
OldStreetSegmentItem::getMemoryUsage() const 
{
   uint32 totalSize = OldRouteableItem::getMemoryUsage()
      - sizeof(OldRouteableItem) + sizeof(OldStreetSegmentItem);
   
   return totalSize;
}

bool
OldStreetSegmentItem::save(DataBuffer* dataBuffer) const
{
   DEBUG_DB(mc2dbg << "      OldStreetSegmentItem::save()" << endl;)

   OldRouteableItem::save(dataBuffer);

   DEBUG_DB(mc2dbg << "         Saving attributes for this item" << endl;)

   dataBuffer->writeNextByte(m_condition);
   dataBuffer->writeNextByte(m_roadClass);
   byte tmpData = m_width & WIDTH_MASK;
   if (m_roundaboutish)
      tmpData |= ROUNDABOUTISH_MASK;
   dataBuffer->writeNextByte(tmpData);
   
   // Store the three flaggs in the streetNumberType-byte on disc
   tmpData = byte(m_streetNumberType) & STREETNUMBERTYPE_MASK;
   if (m_roundabout)
       tmpData |= ROUNDABOUT_MASK;
   if (m_ramp)
       tmpData |= RAMP_MASK;
   if (m_divided)
       tmpData |= DIVIDED_MASK;
   if (m_multiDigitised)
       tmpData |= MULTI_DIGITISED_MASK;
   if (m_controlledAccess)
      tmpData |= CONTROLLED_ACCESS_MASK;
   dataBuffer->writeNextByte(tmpData);

   dataBuffer->writeNextShort(m_leftsideNumberStart);
   dataBuffer->writeNextShort(m_leftsideNumberEnd);

   dataBuffer->writeNextShort(m_rightsideNumberStart);
   dataBuffer->writeNextShort(m_rightsideNumberEnd);
   return (true);
}

char* OldStreetSegmentItem::toString()
{
   char tmpStr[ITEM_AS_STRING_LENGTH];

   // Character that is 'T' if this SSI is a ramp, 'F' otherwise
   char rampVal = 'F';
   if (m_ramp)
      rampVal = 'T';
   
   // Character that is 'T' if this SSI is a roundabout, 'F' otherwise
   char roundaboutVal = 'F';
   if (m_roundabout)
      roundaboutVal = 'T';
   
   // Character that is 'T' if this SSI is divded, 'F' otherwise
   char dividedVal = 'F';
   if (m_divided)
      dividedVal = 'T';
    
   // Character that is 'T' if this SSI is multi digitised, 'F' otherwise
   char multiDigitisedVal = 'F';
   if (m_multiDigitised)
      multiDigitisedVal = 'T';

   // Character that is 'T' if this SSI is a controlled access, 'F' otherwise
   char controlledAccessVal = 'F';
   if (m_controlledAccess)
      controlledAccessVal = 'T';
    
   strcpy(tmpStr, OldRouteableItem::toString());
   sprintf(itemAsString,   "***** StreetSegmentItem\n%s"
                           "   width=%u\n"
                           "   streetNumberType=%u\n"
                           "   leftsideNumberStart=%u\n"
                           "   leftsideNumberEnd=%u\n"
                           "   rightsideNumberStart=%u\n"
                           "   rightsideNumberEnd=%u\n"
                           "   condition=%u\n"
                           "   roadClass=%u\n"
                           "   ramp=%c\n"
                           "   roundabout=%c\n"
                           "   divided=%c\n"
                           "   multiDigitised=%c\n"
                           "   controlledAccess=%c\n",
                           tmpStr,
                           m_width,
                           byte(m_streetNumberType),
                           m_leftsideNumberStart,
                           m_leftsideNumberEnd,
                           m_rightsideNumberStart,
                           m_rightsideNumberEnd,
                           m_condition,
                           m_roadClass,
                           rampVal,
                           roundaboutVal,
                           dividedVal,
                           multiDigitisedVal,
                           controlledAccessVal);
   return itemAsString;
}


bool
OldStreetSegmentItem::createFromDataBuffer(DataBuffer* dataBuffer,
                                        OldGenericMap* theMap)
{
   m_type = ItemTypes::streetSegmentItem;
   if (OldRouteableItem::createFromDataBuffer(dataBuffer, theMap)) {
      // Read the attributes for this OldItem
      m_condition = dataBuffer->readNextByte();
      m_roadClass = dataBuffer->readNextByte();
      byte tmpData = dataBuffer->readNextByte();
      m_width = tmpData & WIDTH_MASK;
      m_roundaboutish = ( (tmpData & ROUNDABOUTISH_MASK) != 0);
       
      tmpData = dataBuffer->readNextByte();
      m_streetNumberType = 
         ItemTypes::streetNumberType(tmpData & STREETNUMBERTYPE_MASK);
      m_roundabout = ( (tmpData & ROUNDABOUT_MASK) != 0);
      m_ramp = ( (tmpData & RAMP_MASK) != 0);
      m_divided = ( (tmpData & DIVIDED_MASK) != 0);
      m_multiDigitised = ( (tmpData & MULTI_DIGITISED_MASK) != 0);
      m_controlledAccess = ( (tmpData & CONTROLLED_ACCESS_MASK) != 0);

      DEBUG_DB( mc2dbg << "   condition=" << (uint32) m_condition << endl);
      DEBUG_DB( mc2dbg << "   roadClass=" << (uint32) m_roadClass << endl);
      DEBUG_DB( mc2dbg << "   width=" << (uint32) m_width << endl);
      DEBUG_DB( mc2dbg << "   streetNumberType=" 
                     << (uint32) m_streetNumberType << "=" << (hex) 
                     << (uint32) m_streetNumberType << (dec)<< endl);

      m_leftsideNumberStart = dataBuffer->readNextShort();
      m_leftsideNumberEnd = dataBuffer->readNextShort();
      DEBUG_DB( mc2dbg << "   leftsideNumberStart=" 
                   << (uint32) m_leftsideNumberStart << endl);
      DEBUG_DB( mc2dbg << "   leftsideNumberEnd=" 
                   << (uint32) m_leftsideNumberEnd << endl);

      m_rightsideNumberStart = dataBuffer->readNextShort();
      m_rightsideNumberEnd = dataBuffer->readNextShort();
      DEBUG_DB( mc2dbg << "   rightsideNumberStart=" 
                   << (uint32) m_rightsideNumberStart << endl);
      DEBUG_DB( mc2dbg << "   rightsideNumberEnd=" 
                   << (uint32) m_rightsideNumberEnd << endl);
      return (true);
   } else {
      return (false);
   }
}

void
OldStreetSegmentItem::writeMifHeader(ofstream& mifFile)
{
   OldItem::writeGenericMifHeader(36, mifFile); // give the sum of ssi + 2*node
   //writeSpecificHeader(mif)
   mifFile << "  CONDITION char(25)\r\n"
           << "  ROAD_CLASS integer(3,0)\r\n"
           << "  WIDTH integer(10,0)\r\n"
           << "  STNBRTYPE integer(3,0)\r\n"
           << "  ROUNDABT char(2)\r\n"
           << "  RBTISH char(2)\r\n"
           << "  RAMP char(2)\r\n"
           << "  DIVIDED char(2)\r\n"
           << "  MULTI char(2)\r\n"
           << "  CTRLACC char(2)\r\n"
           << "  LEFTSTART integer(5,0)\r\n"
           << "  LEFTEND integer(5,0)\r\n"
           << "  RGHTSTART integer(5,0)\r\n"
           << "  RGHTEND integer(5,0)\r\n";
   
   OldNode::printNodeMifHeader(mifFile, 0);
   OldNode::printNodeMifHeader(mifFile, 1);
   
   mifFile << "DATA" << endl;
}
void
OldStreetSegmentItem::printMidMif(ofstream& midFile, ofstream& mifFile, OldItemNames* namePointer)
{
   OldItem::printMidMif(midFile, mifFile, namePointer);

   //Check road condition
   switch(m_condition){
      case ItemTypes::pavedStreet : {
         midFile << ",\"pavedStreet\",";
      } break;
      case ItemTypes::unpavedStreet : {
         midFile << ",\"unpavedStreet\",";
      } break;
      case ItemTypes::poorConditionStreet : {
         midFile << ",\"poorConditionStreet\",";
      } break;
      default : {
         midFile << ",\"Condition unknowned\",";
      }
   }
   
   //Check road class (NavTech classification).
   switch(m_roadClass){
      case ItemTypes::mainRoad : {
         //midFile << "\"mainRoad\",";
         midFile << "0,";
      } break;
      case ItemTypes::firstClassRoad : {
         //midFile << "\"firstClassRoad\",";
         midFile << "1,";
      } break;
      case ItemTypes::secondClassRoad : {
         //midFile << "\"secondClassRoad\",";
         midFile << "2,";
      } break;
      case ItemTypes::thirdClassRoad : {
         //midFile << "\"thirdClassRoad\",";
         midFile << "3,";
      } break;
      case ItemTypes::fourthClassRoad : {
         //midFile << "\"fourthClassRoad\",";
         midFile << "4,";
      } break;
      default : {
         //midFile << "\"Road not classified\",";
         midFile << "-1,";
      }
   }
   
   midFile << int32(m_width) << ",";

   //Check street number type
   switch(m_streetNumberType){
      case ItemTypes::noStreetNumbers : {
         midFile << "\"noStreetNumbers\",";
      } break;
      case ItemTypes::mixedStreetNumbers : {
         midFile << "\"mixedStreetNumbers\",";
      } break;
      case ItemTypes::leftEvenStreetNumbers : {
         midFile << "\"leftEvenStreetNumbers\",";
      } break;
      case ItemTypes::leftOddStreetNumbers : {
         midFile << "\"leftOddStreetNumbers\",";
      } break;
      default : {
         midFile << "\"No such street number type\",";
      }
   }

   //Print bool facts about the street segment.
   midFile << "\""
           << Utility::convertBoolToString(m_roundabout) << "\",\""
           << Utility::convertBoolToString(m_roundaboutish) << "\",\""
           << Utility::convertBoolToString(m_ramp) << "\",\""
           << Utility::convertBoolToString(m_divided) << "\",\""
           << Utility::convertBoolToString(m_multiDigitised) << "\",\""
           << Utility::convertBoolToString(m_controlledAccess) << "\",";

   //Print street numbering.
   midFile << m_leftsideNumberStart << ","
           << m_leftsideNumberEnd << ","
           << m_rightsideNumberStart << ","
           << m_rightsideNumberEnd << ",";
   
   m_node0->printNodeMidData(midFile);
   midFile << ",";
   m_node1->printNodeMidData(midFile);
      
   midFile << endl;
} // printMidMif

void 
OldStreetSegmentItem::setStreetNumberType(ItemTypes::streetNumberType x)
{
   m_streetNumberType = x;
}

void  
OldStreetSegmentItem::setLeftSideNumberStart(uint16 t)
{
   m_leftsideNumberStart = t;
}

void  
OldStreetSegmentItem::setLeftSideNumberEnd(uint16 t)
{
   m_leftsideNumberEnd = t;
}

void  
OldStreetSegmentItem::setRightSideNumberStart(uint16 t)
{
   m_rightsideNumberStart = t;
}

void  
OldStreetSegmentItem::setRightSideNumberEnd(uint16 t)
{
   m_rightsideNumberEnd = t;
}

bool
OldStreetSegmentItem::updateAttributesFromItem(OldItem* otherItem,
                                            bool sameMap)
{
   bool retVal = false;
   mc2dbg4 << "OldStreetSegmentItem::updateAttributesFromItem" << endl;
   if ((otherItem == NULL) ||
       (otherItem->getItemType() != ItemTypes::streetSegmentItem)) {
      return retVal;
   }
   
   // First general attributes such as names and gfxdata
   if (OldItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then routeable item specific attributes
   if (OldRouteableItem::updateAttributesFromItem(otherItem, sameMap))
      retVal = true;
   
   // Then item specific attributes
   OldStreetSegmentItem* otherSsi = (OldStreetSegmentItem*) otherItem;
   if (otherSsi != NULL) {

      // hosenumbers, 4 of them + streetNumberType
      // width
      // roadClass - don't update, connected with zoomlevel
      // condition
      // roundabout
      // roundaboutish
      // ramp
      // divided
      // multidig
      // controlled acces
    
      bool changedStreetNbrs = false;
      if (m_leftsideNumberStart != otherSsi->getLeftSideNbrStart()) {
         m_leftsideNumberStart = otherSsi->getLeftSideNbrStart();
         mc2dbg4 << "changing leftSideStart for ssi " << getID() << endl;
         changedStreetNbrs = true;
      }
      
      if (m_leftsideNumberEnd != otherSsi->getLeftSideNbrEnd()) {
         m_leftsideNumberEnd = otherSsi->getLeftSideNbrEnd();
         mc2dbg4 << "changing leftSideEnd for ssi " << getID() << endl;
         changedStreetNbrs = true;
      }
      
      if (m_rightsideNumberStart != otherSsi->getRightSideNbrStart()) {
         m_rightsideNumberStart = otherSsi->getRightSideNbrStart();
         mc2dbg4 << "changing rightSideStart for ssi " << getID() << endl;
         changedStreetNbrs = true;
      }
      
      if (m_rightsideNumberEnd != otherSsi->getRightSideNbrEnd()) {
         m_rightsideNumberEnd = otherSsi->getRightSideNbrEnd();
         mc2dbg4 << "changing rightSideEnd for ssi " << getID() << endl;
         changedStreetNbrs = true;
      }
      
      if (changedStreetNbrs) {
         // update streetNbrType since some of the houseNbr changed
         ItemTypes::streetNumberType newType = 
            ItemTypes::getStreetNumberTypeFromHouseNumbering(
                  m_leftsideNumberStart, m_leftsideNumberEnd,
                  m_rightsideNumberStart, m_rightsideNumberEnd);
         m_streetNumberType = newType;
         retVal = true;
      }
      
      if (m_width != otherSsi->getWidth()) {
         m_width = otherSsi->getWidth();
         mc2dbg4 << "changing width for ssi " << getID() << endl;
         retVal = true;
      }
      
      if (m_condition != otherSsi->getRoadCondition()) {
         m_condition = otherSsi->getRoadCondition();
         mc2dbg4 << "changing condition for ssi " << getID() << endl;
         retVal = true;
      }
      
      if (m_roundabout != otherSsi->isRoundabout()) {
         m_roundabout = otherSsi->isRoundabout();
         mc2dbg4 << "changing roundabout for ssi " << getID() << endl;
         retVal = true;
      }
      
      if (m_roundaboutish != otherSsi->isRoundaboutish()) {
         m_roundaboutish = otherSsi->isRoundaboutish();
         mc2dbg4 << "changing roundaboutish for ssi " << getID() << endl;
         retVal = true;
      }
      
      if (m_ramp != otherSsi->isRamp()) {
         m_ramp = otherSsi->isRamp();
         mc2dbg4 << "changing ramp for ssi " << getID() << endl;
         retVal = true;
      }
      
      if (m_divided != otherSsi->isDivided()) {
         m_divided = otherSsi->isDivided();
         mc2dbg4 << "changing divided for ssi " << getID() << endl;
         retVal = true;
      }
      
      if (m_multiDigitised != otherSsi->isMultiDigitised()) {
         m_multiDigitised = otherSsi->isMultiDigitised();
         mc2dbg4 << "changing multi dig for ssi " << getID() << endl;
         retVal = true;
      }
      
      if (m_controlledAccess != otherSsi->isControlledAccess()) {
         m_controlledAccess = otherSsi->isControlledAccess();
         mc2dbg4 << "changing controlled access for ssi " << getID() << endl;
         retVal = true;
      }
   }

   return retVal;
}

