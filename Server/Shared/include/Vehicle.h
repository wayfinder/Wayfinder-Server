/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ROUTINGVEHICLE_H
#define ROUTINGVEHICLE_H

#include "ItemTypes.h"

/**
 *   Class representing a vehicle or a pedestrian. 
 *   The vehicle is used to keep track of the current vehicle
 *   used by a route to get to a certain point.
 *   <code>DriverPref</code> keep track of which vehicle transitions
 *   that are possible and also creates the vehicles.
 *   Vehicles should only be created by <code>DriverPref</code>.
 *
 */
class Vehicle {

public:

   /// Constant for nodeRestrictions when all is allowed
   static const uint32 allowAllNodeRestrictions = MAX_UINT32;
   
   /**
    *   Returns the maximum estimated distance to the destination
    *   when this vehicle is allowed to be used. (Typically MAX_UINT32
    *   for a car and a low value for a pedestrian).
    *   @return The maximum estimated distance to the destination
    *           when this vehicle is allowed to be used.
    */
   inline uint32 getMaxAllowedDist() const;

   /**
    *   Returns the name of the vehicle.
    *   @return The name of the vehicle.
    */
   inline const char* getName() const;
   
   /**
    *   Returns the vehicle type mask for the vehicle.
    *   @see ItemTypes.h
    *   @return The vehicle type mask for the vehicle.
    */
   inline uint32 getVehicleMask() const;

   /**
    *   Returns the node restrictions where the vehicle can drive.
    *   @see ItemTypes.h
    *   @return The node restrictions for the vehicle.
    */
   inline uint32 getNodeRestrictions() const;

   /**
    *   Returns the penalty for this type of vehicle. This is a factor
    *   which compares this vehicle to the best one in a driverspref.
    *   The best one should typically have 1 (one) as penalty.
    *   @return The penalty for this type of vehicle.
    */
   inline uint32 getPenalty() const;

   /**
    *   Returns the class of the vehicle. Larger number is better class.
    *   Zero is best class in <code>DriverPref</code>.
    *   @return The class of the vehicle.
    */
   inline int getClass() const;

   /**
    *   Returns <b><code>true</code></b> if the vehicle is allowed on
    *   this street.
    *   @param connRestrictions The connection restrictions.
    *   @param nodeRestrictions The node restriction.
    *   @return <b><code>true</code></b> if the vehicle is allowed here.
    */
   inline bool isAllowed(uint32 connRestrictions,
                         uint32 nodeRestrictions) const;

   /**
    *   Returns the index of the <code>Vehicle</code> in its
    *   <code>DriverPref</code>.
    */
   inline int getIdx() const;

   /**
    *   Returns a pointer to a better vehicle from the DriverPrefs if
    *   it is possible to change from a better vehicle to a worse one.
    *   @return A pointer to a better vehicle if there is any.
    */
   inline Vehicle* getBetter() const;

   /**
    *   Returns a pointer to a worse vehicle from the DriverPrefs
    *   if there is any.
    *   @return A worse vehicle from the DriverPrefs.
    */
   inline Vehicle* getWorse() const;


protected:

   
   /** The maximum est. distance to drive this vehicle (or walk) */
   uint32 m_maximumDistance;
   
   /** The vehicle mask of the vehicle */
   uint32 m_vehicleMask;

   /** The nodes that this vehicle can't drive on */
   uint32 m_nodeRestriction;

   /**
    *  The penalty for using this vehicle ( compared to the best one in
    *   current driverpref)
    */
   uint32 m_penalty;
   
   /**
    *  The class of the vehicle. Higher number means better vehicle.
    *  Best class in a driverpref must be zero.
    */
   int m_class;

   /// A pointer to a better vehicle in the DriverPref.
   Vehicle* m_betterVehicle;

   /// A pointer to a worse vehicle in the DriverPref.
   Vehicle* m_worseVehicle;

   /// The index of the vehicle in the <code>DriverPref</code>.
   int m_idx;
   
   /** The allocated size for the name. Should be enough for anybody */
   static const int c_maxNameSize = 20;
   
   /**
    *  The name of the vehicle. Can be used for debugging. Fixed
    *  size so we don't have to have a destructor
    */
   char m_name[c_maxNameSize];

};

// Implementation of inlined functions

inline Vehicle*
Vehicle::getBetter() const
{
   return m_betterVehicle;
}

inline Vehicle*
Vehicle::getWorse() const
{
   return m_worseVehicle;
}

inline int
Vehicle::getClass() const
{
   return m_class;
}

inline int
Vehicle::getIdx() const
{
   return m_idx;
}

inline const char*
Vehicle::getName() const
{
   return m_name;
}

inline uint32
Vehicle::getVehicleMask() const
{
   return m_vehicleMask;
}

inline bool
Vehicle::isAllowed(uint32 connRestriction,
                   uint32 nodeRestriction) const
{
   // Check the connection. One means we can get through.
   if ( m_vehicleMask & connRestriction ) {
      // Check the node. nodeRestriction = MAX_UINT32 means all is allowed
      return ( m_nodeRestriction == nodeRestriction )
         || ( m_nodeRestriction == Vehicle::allowAllNodeRestrictions );
   } else {
      return false;
   }
}

#endif

