/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSNODE_H
#define GMSNODE_H

#include "OldNode.h"
#include "DataBuffer.h"
#include "OldConnection.h"

class OldGenericMap;

/**
  *   A OldNode with some extra features neede when creating the map.
  *
  */
class GMSNode : public OldNode {
   public:
      /**
        *   Create an empty GMSNode.
        */
	   GMSNode();

      /**
        *   Create an GMSNode with id.
        */
      GMSNode(uint32 id);

      /**
        *   Create an GMSNode with id.
        */
      GMSNode(DataBuffer* dataBuffer, uint32 id);

      /**
        *   Set vehicleRestrictions
        */
      void setVehicleRestrictions(uint32 restr)
      {
         m_vehicleRestrictions = restr;
      }

      /**
        *   Get vehicleRestrictions
        */
      uint32 getVehicleRestrictions()
      {
         return (m_vehicleRestrictions);
      }
      
      /// ID to point feature record
      uint32 pofeID;
      
      /// ID to knot record
      uint32 knotID;

      /// ID:s for segmented attributes
      Vector* dsatIDVec;
         
      /// ID:s for names
      Vector* nameIDVec;

       /** 
        * Returns false if the turn directions of the connections to this
        * node has been set.
        * @return False if turn directions are set for the node.
        */
      bool turnUnSet()
         {return !m_turnsSet; }

      /**
       *  Sets the turnsSet flag to true.
       *  @return true if turnsSet flag was false.
       */
      bool setTurnSet();
      
   private:
      void init();
      
      /** 
        *   Vehicle restrictions -- bit set indicate that the vehicle type
        *   is allowed to enter the streetsegment at this node, 0 that
        *   it not is allowed.
        */
      uint32 m_vehicleRestrictions;

      /// Flagg to determine if turndirectons are set.
      bool m_turnsSet;

};

#endif
