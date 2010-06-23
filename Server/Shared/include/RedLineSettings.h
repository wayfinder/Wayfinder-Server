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

#include <vector>

class Packet;

class RedLineSettings {
public:

   /// Min speed in first and nbr meters in second
   typedef vector<pair<uint32, uint32> > speedVect_t;

   /**
    *    Sets empty settings, but with default speed settings.
    */
   RedLineSettings();
   
   /**
    *    @param include_route True if the route should be included.
    *    @param include_connecting_roads True if connecting roads should
    *                                    be added as well.
    */
   RedLineSettings( bool include_connecting_roads,
                    const speedVect_t& speedvect = c_defaultSpeedVect );

   /**
    *    Sets if the connecting roads should be included.
    */
   void setIncludeConnectingRoads( bool to_inc ) {
      m_incConnecting = to_inc;
   }

   /**
    *  Gets if the connecting roads should be included.
    */
   bool getIncludeConnectingRoads() const {
      return m_incConnecting;
   }
   
   /**
    *    Swaps the speed vector.
    */
   void swapSpeedVect( speedVect_t& new_one ) {
      m_speedVect.swap( new_one );
   }   

  /**
   *     Gets the speed vector.
   */
   const speedVect_t& getSpeedVect() const {
      return m_speedVect;
   }

   /**
    *    Swaps the speed vector for bounding box.
    */
   void swapBBoxSpeedVect( speedVect_t& new_one ) {
      m_bboxSpeedVect.swap( new_one );
   }

   /**
    *    Returns the speed vector for the close to route type
    *    of Redline.
    */
   const speedVect_t& getBBoxSpeedVect() const {
      return m_bboxSpeedVect;
   }
   
   /**
    *    Saves settings to packet.
    */
   int save( Packet* packet, int& pos ) const;

   /**
    *    Loads settings from packet.
    */
   int load( const Packet* packet, int& pos );

   /**
    *    Returns the size of the settings if they are saved in a packet.
    */
   uint32 getSizeInPacket() const;

   /**
    *    Prints the settings on a stream.
    */
   friend ostream& operator<<( ostream& o, const RedLineSettings& settings );

private:
   /**
    *    The default speed vector.
    */
   static speedVect_t c_defaultSpeedVect;

   /// Vector of speed settings
   speedVect_t m_speedVect;
   /// Vector of speed settings for bounding box.
   speedVect_t m_bboxSpeedVect;
   /// True if connecting roads should be included.
   bool m_incConnecting;
};
