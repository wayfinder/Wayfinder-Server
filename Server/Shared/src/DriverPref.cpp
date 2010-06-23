/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "DriverPref.h"
#include "ItemTypes.h"
#include "Packet.h"
#include "StringTable.h"

/**
 *   Hidden class used when creating vehicles. Has some more functions
 *   than the real vehicle.
 *   DriverPref should be the only one capable of creating vehicles for
 *   use in the state machine.
 */
class DriverPrefVehicle : public Vehicle {

public:
   
   /**
    *    Creates a new <code>Vehicle</code>. Typically only used from
    *    DriverPref.
    *    @param name     The name of the vehicle. (for debugging).
    *    @param id       The id of the vehicle inside driverpref.
    *    @param worseID  The id of a vehicle that is slower than this
    *                    one, but could possibly be used on more streets.
    *    @param betterID The id of a vehicle that is faster than this one
    *                    and can be changed to from this one.
    *    @param penalty  The penalty factor of the vehicle compared to the
    *                    fastest one in the driverprefs. Could be revised.
    *    @param vehicleClass Zero for the best vehicle. Less and less for
    *                        less preferred vehicles.
    *    @param maxEstDist The maximum straight line distance to the
    *                      closest final destination where this vehicle can
    *                      be used.
    *    @param nodeRest   Noderestriction needed for the vehicle.
    */
   DriverPrefVehicle(const char* name,
                     int idx,
                     int worseIdx,
                     int betterIdx,
                     uint32 penalty,
                     int vehicleClass,
                     uint32 maxEstDist,
                     uint32 vehicleParam = ItemTypes::pedestrian,
                     uint32 nodeRest = Vehicle::allowAllNodeRestrictions) {
      
      strncpy(m_name, name, Vehicle::c_maxNameSize);
      m_name[Vehicle::c_maxNameSize - 1] = '\0';
      m_idx             = idx;
      m_worseIdx        = worseIdx;
      m_betterIdx       = betterIdx;
      m_penalty         = penalty;
      m_vehicleMask     = vehicleParam;
      m_nodeRestriction = nodeRest;
      m_class           = vehicleClass;
   }

   /**
    *    Returns the worse idx for use by DriverPrefs.
    */
   int getWorseIdx() const {
      return m_worseIdx;
   }

   /**
    *    Returns the better idx for use by DriverPrefs when building
    *    state machine.
    */
   int getBetterIdx() const {
      return m_betterIdx;
   }

   /**
    *    Sets the better vehicle.
    */
   void setBetter(Vehicle* better) {
      m_betterVehicle = better;
   }

   /**
    *    Sets the worse vehicle.
    */
   void setWorse(Vehicle* worse) {
      m_worseVehicle = worse;
   }
   
private:

   /**
    *  Index of worse vehicle. Used by driverprefs to find the right
    *  pointer. Needed since the vehicles can refer to not yet created
    *  vehicles.
    */
   int m_worseIdx;

   /**
    *  Index of better vehicle. Used by driverprefs to find the right
    *  pointer. Needed since the vehicles can refer to not yet created
    *  vehicles.
    */
   int m_betterIdx;
   
};

// NEW

void
DriverPref::deleteVehicles()
{
   m_bestVehicle = m_nonValidVehicle = m_ntStartVehicle = NULL;

   // Delete the vehicles.
   VehicleVector_t::iterator it = m_vehicles.begin();
   while ( it != m_vehicles.end() ) {
      // Delete the vehicle
      delete static_cast<DriverPrefVehicle*>(*it);
      ++it;
   }
   // Empty the map.
   m_vehicles.clear();
}

void
DriverPref::updatePointers(DriverPrefVehicle* vehicles[],
                           int nbrVehicles,
                           int nonvalidIdx,
                           int ntStartIdx)
{
   // Remove old vehicles from the list if there are any.
   deleteVehicles();

   // Later i found out that the array can be copied into the
   // vector in a simpler way.
   
   // Cast to avoid putting DriverPrefVehicle in the .h-file.
#  define GET_VEH(x) (( x >= 0 ) ? ( vehicles[x] ) : NULL)
   // Set the nonvalid vehicle
   m_nonValidVehicle = vehicles[nonvalidIdx];
   // Set the NT start vehicule
   m_ntStartVehicle  = vehicles[ntStartIdx];
   // Put the vehicles into the map and find the best vehicle.
   int32 bestClass = MIN_INT32;
   const Vehicle* bestVehicle = NULL;
   for(int i=0; i < nbrVehicles; ++i ) {
         DriverPrefVehicle* curVeh = vehicles[i];
         // Set the better vehicle pointer - can be NULL
         curVeh->setBetter( GET_VEH(curVeh->getBetterIdx() ) );
         // Set the worse vehicle pointer - can be NULL
         curVeh->setWorse( GET_VEH(curVeh->getWorseIdx() ) );
         // Find the best class for crashing later.
         bestClass = MAX(bestClass, curVeh->getClass());
         if ( curVeh->getClass() == bestClass ) {
            bestVehicle = curVeh;
         }
         // Put it into the real vector.
         m_vehicles.push_back(curVeh);
   }
   // Sanity check. Remove later. Will crash here if it's wrong.
   if ( bestClass != 0 ) {
      mc2log << fatal << "Best vehicle \"" << bestVehicle->getName()
             << "\" has class " << bestClass
             << " != 0. I give up" << endl;
      char* kalle = NULL;
      *kalle = 'l';
   }
   // Sanity check. Remove later.
   for(int i=0; i < (int)(m_vehicles.size()); i++) {
      if ( getVehicle(i)->getIdx() != i ) {
         mc2log << fatal << "Index of \"" << bestVehicle->getName()
                << "\" is " << getVehicle(i)->getIdx()
                << " != " << i << ". I give up" << endl;
         char* kalle = NULL;
         *kalle = 'l';
      }
   }
   m_bestVehicle = bestVehicle;
}

bool
DriverPref::buildStateMachine(uint32 vehicleParam)
{   
   // Penalty for the big vehicle.
   const int carPenalty = 1;
   // Penalty for the walker. Should be possible to specify outside later.
   const int pedPenalty = 10;
   const int maxWalkingDist = 5000;
   // Return NULL if the vehicle is negative or else the vehicle

   const char* bestName = "car";
   // Car is short and nice. Check if it wasn't car
   if ( vehicleParam != ItemTypes::passengerCar ) {
      bestName = StringTable::getString(
         ItemTypes::getVehicleSC(ItemTypes::vehicle_t(vehicleParam)),
         StringTable::ENGLISH);
   }
   if ( vehicleParam == ItemTypes::bicycle )
      bestName = "bike";

   if ( vehicleParam != ItemTypes::pedestrian ) {
      // Not a pedestrian. We'd like to give the user the
      // possibility to drive the "car" as far as possible
      // but also to walk.

      // Manually fill in the noderestrictions
      const uint32 car_node = ItemTypes::noRestrictions;
      const uint32 nt_node  = ItemTypes::noThroughfare;
      // Can walk everywhere when starting
      const uint32 nv_node   = Vehicle::allowAllNodeRestrictions;
      const uint32 nv_conn   = MAX_UINT32; // Maybe pedestrian here?
      
      // Manually count the number of vehicles.
      const int nbrVehicles = 5;
      // NB! Best class should always be zero!
      DriverPrefVehicle* vehicles [] = {
         // name                                   // idx // worse // better
         new DriverPrefVehicle(bestName        ,   0,       3,      -1,
                               // penalty // class // dist // params
                               carPenalty,  0,   MAX_UINT32, vehicleParam,
                               // noderest
                               car_node),
         // The nonvalid vehicle
         //
         // Almost always allowed. Will check if car is allowed too and
         // if it is, we will get into the real car and drive.
         // name                                   // idx // worse // better
         new DriverPrefVehicle("non valid"     ,   1,       -1,       0,
                               // penalty // class // dist // params
                               pedPenalty, -1,  MAX_UINT32, nv_conn,
                               // noderest
                               nv_node),
         // No throughfare entry vehicle
         //
         // When the vehicle isn't allowed the route will turn into
         // a walking route. This should ensure that we do not cross
         // NT-areas with the car.
         // name                                   // idx // worse // better
         new DriverPrefVehicle("no throughfare",   2,       3,      -1,
                               // penalty // class // dist 
                               carPenalty, -2,  MAX_UINT32, vehicleParam,
                               // noderest
                               nt_node),
         // The pedestrian.
         // 
         // name                                   // idx // worse // better
         new DriverPrefVehicle("pedestrian"    ,   3,      -1,      -1,
                               // penalty // class // dist 
                               pedPenalty, -4,  maxWalkingDist,
                               // connparams
                               ItemTypes::pedestrian),
         // The no-throughfare start vehicle.
         //
         // When it is not allowed it will check if the car is allowed
         // If the car isn't allowed either then the car will point to
         // the pedestrian.
         // name                                   // idx // worse // better
         new DriverPrefVehicle("nt-start"    ,     4,       0,       -1,
                               // penalty // class // dist 
                               carPenalty, -3,  MAX_UINT32,
                               // connparams // nodeparams
                               vehicleParam, nt_node),
      };

      // Manually set the non-valid vehicle.
      const int nonvalid = 1;
      const int ntStart  = 4;
      updatePointers(vehicles, nbrVehicles, nonvalid, ntStart);
   } else {
      // We have a pedestrian. One states possible.
      const int nbrVehicles = 1;
      DriverPrefVehicle* vehicles [] = {
         // name                                   // idx // worse // better
         new DriverPrefVehicle("pedestrian"    ,   0,      -1,      -1,
                               // penalty // class // dist 
                                        1,       0,   MAX_UINT32)
      };
      // Manually set the non-valid vehicle.
      const int nonvalid = 0;
      const int ntStart  = 0;
      updatePointers(vehicles, nbrVehicles, nonvalid, ntStart);
   }
   return true;
}

// OLD

DriverPref::DriverPref()
{
   setRoutingCosts(0x00010000);
   setVehicleRestriction(ItemTypes::passengerCar);
   m_time         = 0;
   m_minWaitTime  = 0;
   m_isStartTime  = true;
   m_useUturn     = false;
   m_avoidTollRoads = false;
   m_avoidHighways = false;
}

DriverPref::DriverPref(const DriverPref& other)
{
   deleteVehicles();
   setRoutingCosts( other.getRoutingCosts() );
   setVehicleRestriction(other.getVehicleRestriction());
   m_time           = other.m_time;
   m_minWaitTime    = other.m_minWaitTime;
   m_isStartTime    = other.m_isStartTime;
   m_useUturn       = other.m_useUturn;
   m_avoidTollRoads = other.m_avoidTollRoads;
   m_avoidHighways  = other.m_avoidHighways;
   m_bestVehicle = m_nonValidVehicle = m_ntStartVehicle = NULL;
}

DriverPref::~DriverPref()
{
   deleteVehicles();
}

const DriverPref& DriverPref::operator=(const DriverPref& other)
{
   deleteVehicles();
   setRoutingCosts( other.getRoutingCosts() );
   setVehicleRestriction(other.getVehicleRestriction());
   m_time           = other.m_time;
   m_minWaitTime    = other.m_minWaitTime;
   m_isStartTime    = other.m_isStartTime;
   m_useUturn       = other.m_useUturn;
   m_avoidTollRoads = other.m_avoidTollRoads;
   m_avoidHighways  = other.m_avoidHighways;
   return *this;
}


void DriverPref::dump()
{
   mc2dbg << " cost " << hex << m_cost
          << " vehicle " << m_vehicleParam << dec
          << " time " << m_time
          << " minWaitTime " << m_minWaitTime
          << " isStartTime " << m_isStartTime 
          << " useUturn " << m_useUturn 
          << " avoidTollRoads " << m_avoidTollRoads 
          << " avoidHighways " << m_avoidHighways << endl;
   // And the vehicles.
   mc2dbg << "Vehicles : " << endl;
   for(VehicleVector_t::iterator it = m_vehicles.begin();
       it != m_vehicles.end();
       it++) {
      DriverPrefVehicle* curVeh = static_cast<DriverPrefVehicle*>(*it);
      const char* name = curVeh->getName();
      const DriverPrefVehicle* better = static_cast<const DriverPrefVehicle*>
                                        (curVeh->getBetter());
      const char* betterName = (better != NULL) ? better->getName() : "null";
      int betterID = ( better != NULL ) ? better->getIdx() : -1;
      const DriverPrefVehicle* worse  = static_cast<const DriverPrefVehicle*>
                                        (curVeh->getWorse());
      int worseID = ( worse != NULL ) ? worse->getIdx() : -1;
      const char* worseName =  (worse != NULL) ? worse->getName() : "null";
      mc2dbg << "  Name : " << name << "[" << curVeh->getIdx() << "]" << endl
             << "    Worse  : " << worseName  << "[" << worseID  << "]"
             << endl
             << "    Better : " << betterName << "[" << betterID << "]"
             << endl;
   }
}

void
DriverPref::save( Packet* p, int& pos ) const {
   p->incWriteLong( pos, getVehicleRestriction() );
   p->incWriteLong( pos, getRoutingCosts() );
   p->incWriteLong( pos, useUturn() );
   p->incWriteLong( pos, getTime() );
   p->incWriteLong( pos, getIsStartTime() );
   p->incWriteLong( pos, avoidTollRoads() );
   p->incWriteLong( pos, avoidHighways() );
   p->incWriteLong( pos, getMinWaitTime() );
}

void
DriverPref::load( const Packet* p, int& pos ) {
   setVehicleRestriction( p->incReadLong( pos ) );
   setRoutingCosts( p->incReadLong( pos ) );
   setUturn( p->incReadLong( pos ) );
   setTime( p->incReadLong( pos ) );
   setIsStartTime( p->incReadLong( pos ) );
   setAvoidTollRoads( p->incReadLong( pos ) );
   setAvoidHighways( p->incReadLong( pos ) );
   setMinWaitTime( p->incReadLong( pos ) );
}

uint32
DriverPref::getSize() const {
   return 8*4;
}
