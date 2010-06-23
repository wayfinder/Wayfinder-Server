/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSLANE_H
#define GMSLANE_H

#include "config.h"

// Forward declarations.
class DataBuffer;

/**
 *   Class used for storing information of a lane, of a node
 *
 */
class GMSLane {
 public:

   /**
    * The direction this lane will take you when using it.
    *
    * Should be possible to store in two bytes.
    */
   enum laneDirection_t {
      noCarAllowed = 0, // Not allowed to drive with a car.
      noDirectionIndicated = 1, 
      sharpLeft = 2,
      left = 3,
      slightLeft = 4,
      ahead = 5,
      slightRight =6,
      right = 7,
      sharpRight = 8,
      uTurnLeft = 9,
      uTurnRight = 10,
      onlyOppositeDirection = 11,

      nbrLaneDirections // Keep last.
   };

   /**
    * Lane divider types.
    *
    * Should be possible to store in one byte, i.e. no values over 255.
    */
   enum laneDivider_t {
      noDivider = 0, //  No divider
      interruptedLongLines = 1, //   Interrupted Line with Long Lines
      doubleSolidLine = 2, //   Double Solid Line
      singleSolidLine = 3, //   Single Solid Line
      singleSolidAndInterruptedLine = 4, //   Combination of Single Solid & 
                                    //   Interrupted Line
      interruptedAndSolidLine = 5, // Combination of an Interrupted and a 
                               // Solid Line
      interruptedShortLines = 6, //   Interrupted Line with Short lines 
      tollBooth = 15, // Toll booth
      

      nbrLaneDividerTypes // Keep last.
      
      // if adding more types, add also to the GMSLane::setDataFromGDF
      
   };

   /**
    * Lane types.
    */
   enum laneType_t {
      notSpecified = 0, //  Not Specified (Default Value)
      exitEntranceLane = 2, //   Exit/Entrance Lane
      shoulderEmergencyLane = 3, //   Shoulder Lane/Emergency Lane
      parkingLane = 4, //   Parking Lane
      hovLane = 6, // HOV Lane

      nbrLaneTypes // Keep last.
      
      // if adding more types, add also to the GMSLane::setDataFromGDF
   };


   /**
    * This struct is used for storing information about lanes until the
    * lane dependent validity can be extracted from the GDF name record.
    */
   struct gdfLaneData_t{
      /// Only used whith gdfDirOfTrfcFlow.
      uint32 vehicleTypes;
      /// Raw attribute value from GDF.
      uint32 gdfDirOfTrfcFlow;
      /// True if this lane is an exit of entrance.
      bool exitEntrance;
      /// Raw attribute value from GDF.
      uint32 gdfDirectionCategories;
      /// Raw attribute value from GDF.
      uint32 gdfLaneDividerType;
      /// Raw attribute value from GDF.
      uint32 gdfLaneType;
      
      /// Default contructor,
      gdfLaneData_t() {
         vehicleTypes = 0; 
         gdfDirOfTrfcFlow = MAX_UINT32; // invalid value
         exitEntrance = false;
         gdfDirectionCategories = MAX_UINT32; // invalid value
         gdfLaneDividerType = MAX_UINT32; // invalid value
         gdfLaneType = MAX_UINT32; // invalid value
      };
   }; // struct gdfLaneData_t




   /**
    * Constructor
    */
   GMSLane();
   
   /**
    * Methods for saving and loading from and to DataBuffer.
    */
   //@{
   void save(DataBuffer& dataBuffer) const;
   void load(DataBuffer& dataBuffer);

   /**
    * Returns the maximum number of bytes this object will occupy when saved 
    * in  a data buffer.
    */
   static uint32 sizeInDataBuffer();
   
   
   //@}

   void setDataFromGDF(const gdfLaneData_t& gdfLaneData);

   /// Operator for printing this object for debug pupouse.
   friend ostream& operator<< ( ostream& stream, const GMSLane& gmsLane );


   /** @return Returns the lane divider type of the divider to the left of 
    *          this lane.
    */
   laneDivider_t getDividerType() const;

   /// Sets the lane divider type to the left of this lane to dividerType.
   void setDividerType(laneDivider_t dividerType);
   
   /** @return Returns the lane type of this lane.
    */
   laneType_t getLaneType() const;

   /// Sets the lane type of this lane to laneType.
   void setLaneType(laneType_t laneType);
   
   /**
    * Clears the direction categories of this lane and sets it to 
    * onlyOppositeDirection.
    */
   void setOnlyOppositeDir();

   /**
    * Clears the direction categories of this lane and sets it to 
    * noCarAllowed.
    */
   void setNoCarAllowed();
   
   /**
    * Sets the direction category value. Overwrites any previous value. Don't
    * use this one to add another direction category value, see addDirCatValue.
    */
   void setDirCatValue(laneDirection_t dirCatVal);

   /**
    * Reset the value the lane direction category of this item.
    */
   void clearDirCatValue();

   /**
    * @return Returns true if the direction category indicates that this lane
    *         is only used in opposite direction compared to the node it is 
    *         stored in.
    */
   bool isOppositeDirection() const;

   
   /**
    * @return Returns true if a direction category possible to use when
    *         displaying the lanes in the client is set.
    */
   bool directionUseable();

   /**
    * @return Returns the raw bit field with all direction categories stored 
    *         for this lane. The bits are set according to laneDirection_t.
    */
   uint16 getLaneDirBits() const;

   /**
    * Get the lane bits as in ExpandStringLane.
    */
   uint16 getExpandStringLaneBits() const;

   /**
    * @name Methods used by GDF extraction only.
    */
   //@{
   /**
    * Using vehicle type to set direction category. Used by GDF extraction
    */
   void handleVehicleRestr();

   /**
    * Used by GDF extraction.
    *
    * @return Returns true if the vehicle type indicates that car is allowed
    *         for this lane.
    */
   bool vehicleTypeCarAllowed();
   //@}


   /**
    * Always use this method to add normal direction category values. It also
    * hanldes "not allowed with car".
    */
   void addDirCatValue(laneDirection_t dirCatVal );




 private:
   // Member methods

   /// Init with invalid values.
   void initMembers();

   // Member variables
   
   /**
    * Direction category bit field. That is how you are supposed to travel
    * when using this lane. Could be the combination of many directions.
    *
    * Corresponds to the values of laneDirection_t of which 0 is the lowest 
    * bit.
    */
   uint16 m_laneDirection;

   /**
    * Type of divider between lanes, for the divider to the left of the lane.
    */
   laneDivider_t m_laneDivider;

   /**
    * A bit field identifying the allowed vehicle types using 
    * ItemTypes::vehicle_t
    */
   uint32 m_vehicleTypes;

   /**
    * True if this lane is an exit or entrence lane.
    */
   bool m_exitEntrance;

   /**
    * Type of lane.
    */
   laneType_t m_laneType;
   



}; // GMSLane



#endif // GMSLANE_H
