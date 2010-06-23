/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DRIVER_PREF_RM_H
#define DRIVER_PREF_RM_H

#include "config.h"
#include <iostream>

/**
 * Contains information of the customer preferences and
 * about the vehicle used by the customer. Currently, one of cost A -- D
 * is one and the others hold the value zero. The one carrying cost one
 * indicates what kind of routing will take place (A - distance cost
 * routing, B - time routing, C - time routing with traffic disturbances
 * and D is currently not used). Vehicleparam tells what kind of vehicle
 * to calculate the route for.
 *
 */
class RMDriverPref {
  public:
   
   /**
    * This constructor sets the default driver preferences.
    */
   RMDriverPref();
   
   /**
    * Gets cost A.
    * 
    * @return cost A.
    */
   inline byte getCostA() const;
  
   /**
    * Get cost B.
    * 
    * @return Cost B.
    */
   inline byte getCostB() const;
  
   /**
    * Get cost C.
    *
    * @return Cost C.
    */
   inline byte getCostC() const;
   
   /**
    * Get cost D.
    * 
    * @return Cost D.
    */
   inline byte getCostD() const;
  
   /**
    * Get the cost parameters.
    * 
    * @return The cost parameters.
    */
   inline uint32 getRoutingCosts() const;

   /**
    * Set the cost parameters.
    * 
    * @param cost is the new cost parameter.
    */
   inline void setRoutingCosts( uint32 cost );

   /**
    * Get the vehicle parameters.
    * 
    * @return The vehicle parameters.
    */
   inline uint32 getVehicleRestriction() const;

   /**
    * Set the vehicle parameters.
    * 
    * @param vehicle is the vehicle parameters.
    */
   inline void setVehicleRestriction( uint32 vehicle );

   /**
    * Get the m_startOrEndTime.
    *
    * @return The variable startTime or endTime depending on the meaning
    *         of this variable.
    */
   inline uint32 getTime() const;

   /**
    * Set the m_startOrEndTime.
    *
    * @param time The time to set either as startTime or endTime.
    */
   inline void setTime( uint32 time );

   /**
    * Get the minWaitTime.
    *
    * @return The minimum time to wait for a bus.
    */
   inline uint32 getMinWaitTime() const;
   
   /**
    * Set the minWaitTime.
    *
    * @param minWaitTime The minWaitTime to set.
    */
   inline void setMinWaitTime( uint32 minWaitTime );

   /**
    * Get the isStartTime.
    *
    * @return True if startOrEndTime is startTime, false if
    *         startOrEndTime is entTime.
    */
   inline bool getIsStartTime() const;

   /**
    * Set the isStartTime.
    *
    * @param isStartTime If true, m_startOrEndTime will be used as
    *                    startTime, otherwise as endTime.
    */
   inline void setIsStartTime( bool isStartTime );
   
   /**
    * Returns true if we should use uturn penalty for the origins.
    *
    * @return true is uturn penalty should be used.
    */
   inline bool useUturn() const;

   /**
    * Set true if we should use uturn penalty for the origins.
    *
    * @param uturn should be true if uturn penalty should be used.
    */
   inline void setUturn( bool uturn );

   /**
    * Dumps the data to cout.
    */
   void dump();

   /**
    *   Prints the driverprefs on the ostream.
    */
   friend ostream& operator<<( ostream& stream,
                               const RMDriverPref& pref);
   
  private:
   
   /**
    * Consists of 4 parts holding the entry for the 4 different routing costs.
    */
   int32 m_cost; 

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
};

// ======================================================================
//                                   Implementation of inline functions =
// ======================================================================

inline byte RMDriverPref::getCostA() const 
{
   return ( m_cost >> 24 ) & 0xff;
}


inline byte RMDriverPref::getCostB() const 
{
   return ( m_cost >> 16 ) & 0xff;
}


inline byte RMDriverPref::getCostC() const 
{
   return ( m_cost >> 8 ) & 0xff;
}


inline byte RMDriverPref::getCostD() const 
{
   return m_cost & 0xff;
}

inline uint32 RMDriverPref::getRoutingCosts() const
{
   return m_cost;
}

inline void RMDriverPref::setRoutingCosts( uint32 cost )
{
   m_cost = cost;
}

inline uint32 RMDriverPref::getVehicleRestriction() const
{
   return m_vehicleParam;
}

inline void RMDriverPref::setVehicleRestriction( uint32 vehicle )
{
   m_vehicleParam = vehicle;
}


inline uint32 RMDriverPref::getTime() const
{
   return m_time;
}


inline void RMDriverPref::setTime( uint32 time )
{
   m_time = time;
}

inline uint32 RMDriverPref::getMinWaitTime() const
{
   return m_minWaitTime;
}

inline void RMDriverPref::setMinWaitTime( uint32 minWaitTime )
{
   m_minWaitTime = minWaitTime;
}

inline bool RMDriverPref::getIsStartTime() const
{
   return m_isStartTime;
}

inline void RMDriverPref::setIsStartTime( bool isStartTime )
{
   m_isStartTime = isStartTime;
}

inline bool RMDriverPref::useUturn() const
{
   return m_useUturn;
}

inline void RMDriverPref::setUturn( bool uturn )
{
   m_useUturn = uturn;
}

#endif // DRIVER_PREF_H

