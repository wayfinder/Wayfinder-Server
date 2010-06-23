/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSLane.h"

#include "DataBuffer.h"
#include "MapGenUtil.h"
#include "ExpandStringLane.h"

void
GMSLane::initMembers(){

   // Invalid member variable values.
   m_laneDirection = 0; // As long as we don't find any lane info for car,
   // this value remains, e.g. this is a bus file.
   m_laneDivider = nbrLaneDividerTypes;
   m_vehicleTypes = 0;
   m_exitEntrance = false;

   m_laneType = nbrLaneTypes;
} // initMembers


GMSLane::GMSLane() {
   initMembers();
} // GMSLane default constructor


void
GMSLane::save(DataBuffer& dataBuffer) const
{
   dataBuffer.writeNextLong(m_vehicleTypes);
   dataBuffer.writeNextShort(m_laneDirection);
   dataBuffer.writeNextByte(m_laneDivider);
   dataBuffer.writeNextBool(m_exitEntrance);
   dataBuffer.writeNextLong( m_laneType );

} // save


void
GMSLane::load(DataBuffer& dataBuffer)
{
   m_vehicleTypes = dataBuffer.readNextLong();
   m_laneDirection= static_cast<laneDirection_t>(dataBuffer.readNextShort());
   m_laneDivider = static_cast<laneDivider_t>(dataBuffer.readNextByte());
   m_exitEntrance = dataBuffer.readNextBool();
   m_laneType = static_cast< laneType_t >( dataBuffer.readNextLong() );

} // load



uint32
GMSLane::sizeInDataBuffer()
{
   return 
      4 + // m_vehicleTypes
      2 + // m_laneDirection
      1 + // m_laneDivider
      1 +  // m_exitEntrance
      4; // m_laneType

} // sizeInDataBuffer


void
GMSLane::clearDirCatValue(){
   m_laneDirection = 0;
}


void
GMSLane::setDirCatValue(laneDirection_t dirCatVal){
   clearDirCatValue();
   addDirCatValue(dirCatVal);
}

void
GMSLane::addDirCatValue(laneDirection_t dirCatVal )
{
   if (m_laneDirection & (1 << noCarAllowed) ){
      // Do nothing.
   }
   else {
      m_laneDirection = (m_laneDirection | (1 << dirCatVal));
   }
}


void
GMSLane::setDataFromGDF(const gdfLaneData_t& gdfLaneData)
{      
   if ( (gdfLaneData.gdfDirOfTrfcFlow != MAX_UINT32) &&
        (gdfLaneData.gdfDirOfTrfcFlow != 4) ){
      // This direction of traffic flow indicates that the lane is open
      // on some direction for the accossiated vehicle type.
      if ( ! vehicleTypeCarAllowed() ){
         // We have not previously processed any open dir of traffic flow
         // with car allowed.
         m_vehicleTypes = gdfLaneData.vehicleTypes;
      }
      else if (gdfLaneData.vehicleTypes & 1){
         mc2log << warn << "Double direction of traffic flow for car" << endl;
      }
   }
   if (gdfLaneData.exitEntrance){
      m_exitEntrance = gdfLaneData.exitEntrance;
   }
   if (gdfLaneData.gdfDirectionCategories != MAX_UINT32){
      laneDirection_t dirCat = nbrLaneDirections;

      if ( gdfLaneData.gdfDirectionCategories == 0 ){
         dirCat = noDirectionIndicated;
      }
      else if ( gdfLaneData.gdfDirectionCategories & 1 ){
         dirCat = ahead;
      }
      else if ( gdfLaneData.gdfDirectionCategories & 2 ){
         dirCat = slightRight;
      }         
      else if ( gdfLaneData.gdfDirectionCategories & 4){
         dirCat = right;
      }         
      else if ( gdfLaneData.gdfDirectionCategories & 8 ){
         dirCat = sharpRight;
      }         
      else if ( gdfLaneData.gdfDirectionCategories & 16 ){
         dirCat = uTurnLeft;
      }         
      else if ( gdfLaneData.gdfDirectionCategories & 32 ){
         dirCat = sharpLeft;
      }         
      else if ( gdfLaneData.gdfDirectionCategories & 64 ){
         dirCat = left;
      }         
      else if ( gdfLaneData.gdfDirectionCategories & 128 ){
         dirCat = slightLeft;
      }         
      else if ( gdfLaneData.gdfDirectionCategories & 256 ){
         dirCat = uTurnRight;
      }     
      addDirCatValue(dirCat);
      
   }
   if (gdfLaneData.gdfLaneDividerType != MAX_UINT32){
      if ( gdfLaneData.gdfLaneDividerType >= nbrLaneDividerTypes ){
         mc2log << error << "Invalid value of lane divider from GDF: " 
                   << gdfLaneData.gdfLaneDividerType << endl;
         MC2_ASSERT(false);
      } else {
         uint32 type = uint32(gdfLaneData.gdfLaneDividerType);
         if ( (type == 0) ||
              (type == 1) ||
              (type == 2) ||
              (type == 3) ||
              (type == 4) ||
              (type == 5) ||
              (type == 6) ||
              (type == 15) ) {
            // ok, values
            // Need to be kept in sync with enum
         } else {
            mc2log << error << "Invalid value of lane divider from GDF: " 
                      << gdfLaneData.gdfLaneType << endl;
            MC2_ASSERT(false);
         }
      }
      m_laneDivider = 
         static_cast<laneDivider_t>(gdfLaneData.gdfLaneDividerType);
   }
   if (gdfLaneData.gdfLaneType != MAX_UINT32){
      if ( gdfLaneData.gdfLaneType >= nbrLaneTypes ){
         mc2log << error << "Invalid value of lane type from GDF: " 
                   << gdfLaneData.gdfLaneType << endl;
         MC2_ASSERT(false);
      } else {
         uint32 type = uint32(gdfLaneData.gdfLaneType);
         if ( (type == 0) ||
              (type == 2) ||
              (type == 3) ||
              (type == 4) ||
              (type == 6) ) {
            // ok, values
            // Need to be kept in sync with enum
         } else {
            mc2log << error << "Invalid value of lane type from GDF: " 
                      << gdfLaneData.gdfLaneType << endl;
            MC2_ASSERT(false);
         }
      }
      m_laneType = 
         static_cast<laneType_t>(gdfLaneData.gdfLaneType);
   }
   
} // setDataFromGDF


GMSLane::laneDivider_t 
GMSLane::getDividerType() const
{
   return m_laneDivider;
} // getDividerType


void
GMSLane::setDividerType(laneDivider_t dividerType)
{
   m_laneDivider = dividerType;
} // setDividerType

GMSLane::laneType_t
GMSLane::getLaneType() const
{
   return m_laneType;
} // getLaneType


void
GMSLane::setLaneType(laneType_t laneType)
{
   m_laneType = laneType;
} // setLaneType

void
GMSLane::setOnlyOppositeDir()
{
   clearDirCatValue();
   addDirCatValue( onlyOppositeDirection );
} // setOnlyOppositeDir


bool 
GMSLane::vehicleTypeCarAllowed(){
   return (m_vehicleTypes & 1);
}

bool 
GMSLane::isOppositeDirection() const {
   return (m_laneDirection & (1 << onlyOppositeDirection) );
}

uint16
GMSLane::getLaneDirBits() const {
   return m_laneDirection;

}

uint16
GMSLane::getExpandStringLaneBits() const {
   uint16 res = 0;

   if ( (getLaneDirBits() & (1<< noCarAllowed)) != 0 ||
        // hov, parking and emergency lanes should not be drivable
        getLaneType() == hovLane ||
        getLaneType() == parkingLane ||
        getLaneType() == shoulderEmergencyLane ) {
      res |= ExpandStringLane::not_car_lane;
   }
   // Same bit just offseted one bit
   // Remove onlyOppositeDirection
   res |= (getLaneDirBits() & ~(1 << onlyOppositeDirection)) >> 1;

   // Readd onlyOppositeDirection
   if ( isOppositeDirection() ) {
      res |= ExpandStringLane::opposite_direction;
   }

   return res;
}

void
GMSLane::setNoCarAllowed()
{
   clearDirCatValue();
   addDirCatValue( noCarAllowed );
} // setNoCarAllowed


void
GMSLane::handleVehicleRestr(){
   if ( !(vehicleTypeCarAllowed())){
      setNoCarAllowed();
   }
}


ostream& operator<< ( ostream& stream, const GMSLane& gmsLane )
{
   /*MC2String dirBitStr;
   vector<bool> dirBits = uint16ToBitField(gmsLane.m_laneDirection);
   for (uint32 i=0; i<dirBits.size; i++){
      if ( dirBits[i] ){
         dirBitStr.push_back('1');
      }
      else {
         dirBitStr.push_back('0');
      }
   }
   */
   return stream << "dir:" 
                 << MapGenUtil::intToBitFieldStr(gmsLane.m_laneDirection, 16)
                 << " (" << hex << "0x" <<gmsLane.m_laneDirection << dec << ")"
                 << ", div:" << gmsLane.m_laneDivider
                 << ", vt:" 
                 << MapGenUtil::intToBitFieldStr(gmsLane.m_vehicleTypes, 32)
                 << ", exEnt:" << gmsLane.m_exitEntrance 
      ;



} // operator<<

bool
GMSLane::directionUseable(){
   // If noCarAllowed is set, we should not indicate it as unusable direction
   // because this is probably a bus file or similar, and it should be
   // indicated to the user with an x or similar.
   //if ( m_laneDirection == (1<< noCarAllowed) ){
   //   return false;
   //}
   if ( m_laneDirection == (1<< noDirectionIndicated) ){
      return false;
   }
   else if ( m_laneDirection == 0 ){
      return false;
   }
   return true;

} // directionUseable
