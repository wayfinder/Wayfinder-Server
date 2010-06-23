/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "GMSNode.h"
#include "OldGenericMap.h"

GMSNode::GMSNode() 
   : OldNode()
{
   init();
}

GMSNode::GMSNode(uint32 nodeID) 
   : OldNode(nodeID)
{
   init();
}

GMSNode::GMSNode(DataBuffer* dataBuffer, uint32 nodeID) 
   : OldNode(dataBuffer, nodeID, false)
{
   init();
   createFromDataBuffer(dataBuffer, NULL);
}

void
GMSNode::init()
{
   m_majorRoad = false;
   m_entryRestrictions = ItemTypes::noRestrictions;
   m_level = 0;
   m_roadToll = false;
   m_maximumWeight = 0x7;
   m_maximumHeight = 0x3;
   m_speedLimit = MAX_BYTE;
   m_junctionType = ItemTypes::normalCrossing;
   
   pofeID = 0;
   knotID = 0;
   dsatIDVec = NULL;
   nameIDVec = NULL;
   m_vehicleRestrictions = MAX_UINT32;
   m_turnsSet = false;

   m_entryConnections = NULL;
   m_nbrEntryConnections = 0;
}

bool
GMSNode::setTurnSet()
{
   bool ok = !m_turnsSet;
   m_turnsSet= true ;
   return ok;
}
