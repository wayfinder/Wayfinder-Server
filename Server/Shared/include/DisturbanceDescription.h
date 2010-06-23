/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef DISTURBANCE_DESCRITOPTION_H
#define DISTURBANCE_DESCRITOPTION_H

#include "config.h"

#include "IDPairVector.h"
#include "MC2String.h"
#include "TrafficDataTypes.h"

class Packet;

/**
 *   Moved from DisturbanceInfoPacket
 */
class DisturbanceDescription {
public: 

   DisturbanceDescription();

   DisturbanceDescription(IDPair_t distPoint);

   DisturbanceDescription(IDPair_t splitPoint,
                          IDPair_t distPoint,
                          IDPair_t joinPoint);
   
   DisturbanceDescription(IDPair_t splitPoint,
                          IDPair_t distPoint,
                          IDPair_t joinPoint,
                          TrafficDataTypes::disturbanceType type,
                          uint32 distID,
                          const char* comment);

   bool isDetour() const ;
   
   void setDescription(TrafficDataTypes::disturbanceType type,
                       uint32 distID,
                       const char* comment);
   
   bool isDescriptionSet() const ;

   uint32 getMapID() const
      { return m_distPoint.getMapID(); }
   
   uint32 getNodeID() const
      { return m_distPoint.getItemID(); }

   /**
    * Write this DisturbanceDescription as a byte buffer.
    *
    * @param p    The packet where the DisturbanceDescription will
    *             be saved.
    * @param pos  Position in p, will be updated when calling this 
    *             method.
    */
   uint32 save( Packet* p, int& pos ) const;

   /**
    * Get the size of this when stored in a byte buffer.
    *
    * @return  The number of bytes this will use when stored 
    *          in a byte buffer.
    */
   uint32 getSizeAsBytes() const;

   /**
    * Set all members with data from the buffer.
    *
    * @param p    The packet where the DisturbanceDescription will be
    *             loaded from.
    * @param pos  Position in p, will be updated when calling this 
    *             method.
    */
   void load( const Packet* p, int& pos );
   
private:
   ostream& dump( ostream& o ) const {
      o << "DisturbanceDescription: " << m_distPoint << " : "
        << m_splitPoint << " -> " <<  m_joinPoint << ", typ = "
        << uint32( m_type ) << " distid = " << m_distID
        << " comment = " << MC2CITE( m_comment ) << endl;
      return o;
   }
   
   friend ostream& operator<<( ostream& o,
                               const DisturbanceDescription& dist ) {
      return dist.dump(o);
   }
public:
   
  //void set(:
   IDPair_t m_splitPoint;
   IDPair_t m_distPoint;
   IDPair_t m_joinPoint;
   TrafficDataTypes::disturbanceType m_type;
   uint32 m_distID;
   MC2String m_comment;
};

class DisturbanceDescriptionVector{
   public : 
   bool addDescription(uint32 mapID, uint32 nodeID,
                       TrafficDataTypes::disturbanceType type,
                       uint32 distID,
                       const char* comment);
   
   vector<DisturbanceDescription*> m_disturbances;

};


#endif
