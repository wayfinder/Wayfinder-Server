/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DRIVER_PREF_4711_H
#define DRIVER_PREF_4711_H

#include "Types.h"
#include "Vehicle.h"

// NEW
#include <vector>

class DriverPrefVehicle;
class Packet;

/**
 * Contains information of the customer preferences and
 * about the vehicle used by the customer. Currently, one of cost A -- D
 * is one and the others hold the value zero. The one carrying cost one
 * indicates what kind of routing will take place (A - distance cost
 * routing, B - time routing, C - time routing with traffic disturbances
 * and D is currently not used). Vehicleparam tells what kind of vehicle
 * to calculate the route for.
 *
 * DriverPref also contains information about which vehicle transitions
 * that are possible. For example if you drive with a car then you can
 * change to walking, but not the other way around.
 *
 * TODO: Add the vehicle for starting in a NT-area. We already have the
 *       one for driving into a no throughfare area.
 * TODO: Clean out the old vehicleParam which isn't needed when we use
 *       <code>Vehicle</code>.
 *
 */
class DriverPref {
public:
   
   /**
    *   This constructor sets the default driver preferences.
    */
   DriverPref();

   /**
    *   Copy constructor.
    */
   DriverPref(const DriverPref& other);

   /**
    *   The destructor.
    */
   ~DriverPref();

   /**
    * Assignment operator.
    */
   const DriverPref& operator=(const DriverPref& other);
   
   /**
    *   Gets cost A (distance).
    * 
    *   @return cost A.
    */
   inline byte getCostA() const;
  
   /**
    *   Get cost B (time).
    * 
    *   @return Cost B.
    */
   inline byte getCostB() const;
  
   /**
    *   Get cost C (time with disturbance).
    *
    *   @return Cost C.
    */
   inline byte getCostC() const;
   
   /**
    *   Get cost D ( Currently not used ).
    * 
    *   @return Cost D.
    */
   inline byte getCostD() const;
  
   /**
    *   Get the cost parameters, i.e.
    *   <code>A &lt;&lt; 24 | B &lt;&lt; 16 | C &lt;&lt; 8 | D</code>
    * 
    *   @return The cost parameters.
    */
   inline uint32 getRoutingCosts() const;

   /**
    *   Set the cost parameters.
    *   @see getRoutingCosts.
    *   @param cost is the new cost parameter.
    */
   inline void setRoutingCosts( uint32 cost );

   /**
    *   Set cost parameters.
    *   @param costA Weight for distance.
    *   @param costB Weight for time.
    *   @param costC Weight for time including disturbances.
    *   @param costD Not defined.
    */
   inline void setRoutingCosts( byte costA,
                                byte costB,
                                byte costC,
                                byte costD);
   
   /**
    *   Get the vehicle parameters.
    *   @see ItemTypes::vehicle_t.
    *   @return The vehicle parameters.
    */
   inline int32 getVehicleRestriction() const;

   /**
    *   Set the vehicle parameters. The real parameters are in
    *   the <code>Vehicles</code>, though.
    *   Currently builds two default state diagrams. One for pedestrians
    *   where it is only possible to walk and one for other vehicles with
    *   the option to walk when reaching a non-valid area.
    *   @see ItemTypes::vehicle_t.
    *   @param vehicle is the vehicle parameters.
    */
   inline void setVehicleRestriction( uint32 vehicle );

   /**
    *   Get the m_startOrEndTime.
    *
    *   @return The variable startTime or endTime depending on the meaning
    *           of this variable.
    */
   inline uint32 getTime() const;

   /**
    *   Set the m_startOrEndTime.
    *
    *   @param time The time to set either as startTime or endTime.
    */
   inline void setTime( uint32 time );

   /**
    *   Get the minWaitTime.
    *
    *   @return The minimum time to wait for a bus.
    */
   inline uint32 getMinWaitTime() const;
   
   /**
    *   Set the minWaitTime.
    *
    *   @param minWaitTime The minWaitTime to set.
    */
   inline void setMinWaitTime( uint32 minWaitTime );

   /**
    *   Get the isStartTime.
    *
    *   @return True if startOrEndTime is startTime, false if
    *           startOrEndTime is entTime.
    */
   inline bool getIsStartTime() const;

   /**
    *   Set the isStartTime.
    *
    *   @param isStartTime If true, m_startOrEndTime will be used as
    *                      startTime, otherwise as endTime.
    */
   inline void setIsStartTime( bool isStartTime );
   
   /**
    *   Returns true if we should use uturn penalty for the origins.

    *   @return true is uturn penalty should be used.
    */
   inline bool useUturn() const;

   /**
    *   Set true if we should use uturn penalty for the origins.

    *   @param uturn should be true if uturn penalty should be used.
    */
   inline void setUturn( bool uturn );


   /**
    * Returns true if to avoid toll-roads.
    */
   bool avoidTollRoads() const;


   /**
    * Returns true if to avoid highways..
    */
   bool avoidHighways() const;


   /**
    * Sets if to avoid toll-roads.
    */
   void setAvoidTollRoads( bool avoidTollRoads );


   /**
    * Sets if to avoid highways.
    */
   void setAvoidHighways( bool avoidHighways );


   /**
    *   Dumps the data to mc2dbg.
    */
   void dump();

   /**
    *   Returns the best vehicle allowed to drive on the connection
    *   considering the drivers preferences, the current vehicle and
    *   the restrictions for the connection and node to drive into.
    *   For example: If you aren't allowed to drive on a connection and
    *   you are allowed to walk and the driverPref allows it then the
    *   function will return a <code>Vehicle</code> for walking, but
    *   if you are walking, then you cannot get the driving
    *   <code>Vehicle</code>.
    *   @param curVehicle The current vehicle used.
    *                     Cannot be <code>NULL</code>.
    *   @param connRestrictions The connection restrictions to the next node.
    *   @param nextNodeRestrictions The restrictions in the next node.
    *   @return A suitable vehicle considering the indata and the prefs.
    */ 
   inline const Vehicle* getAllowedVehicle( const Vehicle* curVehicle,
                                            uint32 connRestrictions,
                                            uint32 nextNodeRestrictions) const;

   /**
    *    Returns the best vehicle class for this DriverPref.
    *    NB! Always returns zero.
    *    @return The best vehicle class avaliable in the DriverPref.
    *            I.e. zero.
    */
   inline int getBestVehicleClass() const;

   /**
    *    Returns the best vehicle in this driverpref.
    *    @return The best vehicle in the driverpref.
    */
   inline const Vehicle* getBestVehicle() const;

   /**
    *    Gets the number of vehicles in this DriverPref.
    *    @return The number of vehicles in this DriverPref.
    */
   inline int getNbrVehicles() const;
   
   /**
    *    Returns the vehicle with index <code>i</code>.
    *    (Probably used by the RouteModule).
    */
   inline const Vehicle* getVehicle(int i) const;

   /**
    * Saves VehicleRestriction, RoutingCosts, Uturn, IsStartTime, 
    * startOrEndTime, avoidTollRoads, avoidHighways and MinWaitTime.
    */
   void save( Packet* p, int& pos ) const;

   /**
    * Loads from a packet.
    * @see save
    */
   void load( const Packet* p, int& pos );

   /**
    * The size when saved into a Packet.
    */
   uint32 getSize() const;

private:
   

   /**
    *    Helper function for getAllowedVehicle.
    *    @param curVehicle The currently used vehicle.
    *    @param connRestrictions The connection restrictions to the next node.
    *    @param nextNodeRestrictions The restrictions in the next node.
    *    @return The best allowed vehicle among the ones that are worse
    *            than curVehicle.
    */
   inline const Vehicle* getWorseAllowed( const Vehicle* curVehicle,
                                          uint32 connRestrictions,
                                          uint32 nextNodeRestrictions) const;

   /**
    *    Helper function for getAllowedVehicle.
    *    @param curVehicle The currently used vehicle.
    *    @param connRestrictions The connection restrictions to the next node.
    *    @param nextNodeRestrictions The restrictions in the next node.
    *    @return The best allowed vehicle among the ones that are better
    *            than curVehicle.
    */
   inline const Vehicle* getBestAllowed( const Vehicle* curVehicle,
                                         uint32 connRestrictions,
                                         uint32 nextNodeRestrictions) const;


   /**
    *    Moves indexed <code>Vehicles</code> from temporary array
    *    to the real storage. Also updates the pointers to worse and
    *    better vehicles.
    *    @param vehicles    An array of vehicles to be transformed.
    *    @param nbrVehicles The size of <code>vehicles</code>.
    *    @param nonvalidIdx The idx of the Vehicle to be used as nonvalid.
    *    @param ntStartIdx  The idx of the Vehicle to be used when starting
    *                       in a NT-area.
    */
   void updatePointers(DriverPrefVehicle* vehicles[],
                       int nbrVehicles,
                       int nonvalidIdx,
                       int ntStartIdx);
   
   /**
    *    Builds the state machine for the <code>Vehicle</code> transitions.
    *    For now only takes the vehicleParams as inparameter. If the vehicle
    *    is something else than a pedestrian a state machine that goes from
    *    car-type vehicle to pedestrian when a non-valid street is reached
    *    and from car-type vehicle to a special no-throughfare vehicle when
    *    a no-throughfare street is reached.
    */
   bool buildStateMachine(uint32 vehicleParam);

   /**
    *    Deletes the current vehicles of this <code>DriverPref</code> and
    *    empties the list of vehicles.
    */
   void deleteVehicles();
   
   /**
    *    Gets the vehicle to be used when starting in non-valid area.
    *    @return The vehicle to be used when we start where we cannot drive.
    */
   inline const Vehicle* getNonValidVehicle() const;

   /**
    *    Gets the vehicle to be used when starting in a no-throughfare
    *    area.
    *    @return The vehicle to be used when routing starts in a NT-area.
    */
   inline const Vehicle* getNTStartVehicle() const;
   
   /**
    *    Consists of 4 parts holding the entry for the 4
    *    different routing costs (A,B,C and D).
    */
   int32 m_cost; 

   /**
    *    To avoid to many shifts the cost A is stored preshifted here.
    */
   byte m_costA;
   
   /**
    *    To avoid to many shifts the cost B is stored preshifted here.
    */
   byte m_costB;
   
   /**
    *    To avoid to many shifts the cost C is stored preshifted here.
    */
   byte m_costC;
   
   /**
    *    To avoid to many shifts the cost D is stored preshifted here.
    */
   byte m_costD;
   
   /**
    * This variable is a bitfield containing information 
    * on the type of vehicle used by the customer. It is
    * to be ANDed with the connection information.
    *
    * @see ItemTypes.
    */
   int32 m_vehicleParam;

   /**
    * The time associated with the routing (start time or end time for the
    * route. Whether it is starttime or not is determined by the bool
    * isStartTime. This member does not always have a meaning.
    */
   uint32 m_time;

   /**
    * The minimum time to wait at a bus stop. Not used if not routing
    * buses.
    */
   uint32 m_minWaitTime;

   /**
    * Tells if startOrEndTime is used as startTime or EndTime (if that
    * variable) is used. True means startTime, false means endTime.
    */
   bool m_isStartTime;

   /**
    * Tells if we should penalise uturns or not.
    */
   bool m_useUturn;


   /**
    * Tells if we should penalise toll-roads or not.
    */
   bool m_avoidTollRoads;

   /**
    * Tells if we should penalise highways or not.
    */
   bool m_avoidHighways;


   // NEW!!

   /**
    *   Vector containing the vehicles for this <code>DriverPref</code>.
    *   Used mostly in the destructor.
    */
   typedef vector<Vehicle*> VehicleVector_t;
   
   /**
    *   Vector containing the vehicles for this <code>DriverPref</code>.
    *   Used mostly in the destructor.
    */
   VehicleVector_t m_vehicles;

   /**
    *   A pointer to the best vehicle in this <code>DriverPref</code>.
    */
   const Vehicle* m_bestVehicle;

   /**
    *   A pointer to the non-valid vehicle of this <code>DriverPref</code>.
    */
   const Vehicle* m_nonValidVehicle;

   /**
    *   A pointer to the vehicle to be used when starting in an NT-area.
    */
   const Vehicle* m_ntStartVehicle;
};

// ======================================================================
//                                   Implementation of inline functions =
// ======================================================================

inline byte
DriverPref::getCostA() const 
{
   return m_costA;
}

inline byte
DriverPref::getCostB() const 
{
   return m_costB;
}


inline byte
DriverPref::getCostC() const 
{
   return m_costC;
}


inline byte
DriverPref::getCostD() const 
{
   return m_costD;
}

inline uint32 DriverPref::getRoutingCosts() const
{
   return m_cost;
}

inline void DriverPref::setRoutingCosts( uint32 cost )
{
   m_cost = cost;
   m_costA = ( m_cost >> 24 ) & 0xff;
   m_costB = ( m_cost >> 16 ) & 0xff;
   m_costC = ( m_cost >>  8 ) & 0xff;
   m_costD = ( m_cost >>  0 ) & 0xff;
}

inline void DriverPref::setRoutingCosts( byte costA,
                                         byte costB,
                                         byte costC,
                                         byte costD)
{
   m_costA = costA;
   m_costB = costB;
   m_costC = costC;
   m_costD = costD;
   m_cost = uint32(costA) << 24 |
            uint32(costB) << 16 |
            uint32(costC) <<  8 |
            uint32(costD) <<  0;
}

inline int32 DriverPref::getVehicleRestriction() const
{
   return m_vehicleParam;
}

inline void DriverPref::setVehicleRestriction( uint32 vehicle )
{
   m_vehicleParam = vehicle;
   buildStateMachine(vehicle);
}


inline uint32 DriverPref::getTime() const
{
   return m_time;
}


inline void DriverPref::setTime( uint32 time )
{
   m_time = time;
}

inline uint32 DriverPref::getMinWaitTime() const
{
   return m_minWaitTime;
}

inline void DriverPref::setMinWaitTime( uint32 minWaitTime )
{
   m_minWaitTime = minWaitTime;
}

inline bool DriverPref::getIsStartTime() const
{
   return m_isStartTime;
}

inline void DriverPref::setIsStartTime( bool isStartTime )
{
   m_isStartTime = isStartTime;
}

inline bool DriverPref::useUturn() const
{
   return m_useUturn;
}

inline void DriverPref::setUturn( bool uturn )
{
   m_useUturn = uturn;
}


inline bool 
DriverPref::avoidTollRoads() const {
   return m_avoidTollRoads;
}


inline bool 
DriverPref::avoidHighways() const {
   return m_avoidHighways;
}


inline void 
DriverPref::setAvoidTollRoads( bool avoidTollRoads ) {
   m_avoidTollRoads = avoidTollRoads;
}


inline void 
DriverPref::setAvoidHighways( bool avoidHighways ) {
   m_avoidHighways = avoidHighways;
}


inline int DriverPref::getBestVehicleClass() const
{
   return 0;
}

inline int
DriverPref::getNbrVehicles() const
{
   return m_vehicles.size();
}

inline const Vehicle*
DriverPref::getBestVehicle() const
{
   return m_bestVehicle;
}

inline const Vehicle*
DriverPref::getVehicle(int i) const
{
   return m_vehicles[i];
}

inline const Vehicle*
DriverPref::getNonValidVehicle() const
{
   return m_nonValidVehicle;
}

inline const Vehicle*
DriverPref::getNTStartVehicle() const
{
   return m_ntStartVehicle;
}

inline const Vehicle*
DriverPref::getWorseAllowed( const Vehicle* curVehicle,
                             uint32 connRestrictions,
                             uint32 nextNodeRestrictions) const
{
   // Check for another (worse) vehicle which is allowed.
   Vehicle* worseVehicle = curVehicle->getWorse();
   while ( worseVehicle != NULL ) {
      // There is a slower vehicle
      if ( worseVehicle->isAllowed(connRestrictions,
                                   nextNodeRestrictions) ) {
         // The slower vehicle is allowed.
         // Return it since the next one is even worse.
         return worseVehicle;
      }
      // Try with an even worse vehicle.
      worseVehicle = worseVehicle->getWorse();
   }
   // If we are here, we couldn't find any suitable vehicle.
   return worseVehicle; // == NULL!
}

inline const Vehicle*
DriverPref::getBestAllowed( const Vehicle* curVehicle,
                            uint32 connRestrictions,
                            uint32 nextNodeRestrictions) const
{
   const Vehicle* bestVehicle = curVehicle;
   const Vehicle* betterVehicle = curVehicle->getBetter();
   while ( betterVehicle != NULL ) {
      // We have a better vehicle
      if ( betterVehicle->isAllowed(connRestrictions,
                                    nextNodeRestrictions) ) {
         // The better vehicle is allowed
         bestVehicle = betterVehicle;
      }
      // Try to get an even better vehicle.
      betterVehicle = betterVehicle->getBetter();
   }
   return bestVehicle;
}

inline const Vehicle*
DriverPref::getAllowedVehicle( const Vehicle* curVehicle,
                                      uint32 connRestrictions,
                                      uint32 nextNodeRestrictions) const
{
   if ( curVehicle->isAllowed(connRestrictions,
                              nextNodeRestrictions) ) {
      // This vehicle is allowed.
      const Vehicle* betterVehicle = curVehicle->getBetter();
      if ( betterVehicle != NULL ) {
         // The vehicle is not the best vehicle, though.
         // Check if a better vehicle is allowed.
         const Vehicle* bestVehicle = curVehicle;
         while ( betterVehicle != NULL ) {
            // We have a better vehicle
            if ( betterVehicle->isAllowed(connRestrictions,
                                          nextNodeRestrictions) ) {
               // The better vehicle is allowed
               bestVehicle = betterVehicle;
            }
            // Try to get an even better vehicle.
            betterVehicle = betterVehicle->getBetter();
         }
         return bestVehicle;
      } else {
         // Already using the best vehicle
         return curVehicle;
      }
   } else {
      // This vehicle is NOT allowed.
      // Check for another (worse) vehicle which is allowed.
      const Vehicle* worseVehicle = curVehicle->getWorse();      
      while ( worseVehicle != NULL ) {
         // There is a slower vehicle
         if ( worseVehicle->isAllowed(connRestrictions,
                                      nextNodeRestrictions) ) {
            // The slower vehicle is allowed.
            // Return it since the next one is even worse.
            return worseVehicle;
         }
         // Try with an even worse vehicle.
         worseVehicle = worseVehicle->getWorse();
      }
      // If we are here, we couldn't find any suitable vehicle.
      return worseVehicle; // == NULL!
   }
}

#endif // DRIVER_PREF_H

