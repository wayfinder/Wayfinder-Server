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

#include "GfxData.h"
#include "GfxUtility.h"
#include "AlteredGfxList.h"


// ================== AlteredGfxLink ===================

AlteredGfxLink::AlteredGfxLink(uint32 id)
{
   m_id          = id;
   m_removeFirst = false;
   m_removeLast  = false;

   m_newFirstLat = MAX_INT32;
   m_newFirstLon = MAX_INT32;
   
   m_newLastLat  = MAX_INT32;
   m_newLastLon  = MAX_INT32;
}

void
AlteredGfxLink::addFirstCoord(int32 lat, int32 lon)
{
   m_removeFirst = true;
   m_newFirstLat = lat;
   m_newFirstLon = lon;
}

void
AlteredGfxLink::addLastCoord(int32 lat, int32 lon)
{
   m_removeLast = true;
   m_newLastLat = lat;
   m_newLastLon = lon;
}
   
bool
AlteredGfxLink::getFirstCoord(int32 &lat, int32 &lon)
{
   if((m_newFirstLat != MAX_INT32) && (m_newFirstLon != MAX_INT32))
   {
      lat = m_newFirstLat;
      lon = m_newFirstLon;      
      return true;
   }
   return false;
}

bool
AlteredGfxLink::getLastCoord(int32 &lat, int32 &lon)
{
   if((m_newLastLat != MAX_INT32) && (m_newLastLon != MAX_INT32))
   {
      lat = m_newLastLat;
      lon = m_newLastLon;
      return true;
   }
   return false;

}


// ================== AlteredGfxList ===================

AlteredGfxLink*
AlteredGfxList::getItemInList(uint32 itemId)
{
   AlteredGfxLink* link = static_cast<AlteredGfxLink*>(first());
   while(link != NULL){
      if(REMOVE_UINT32_MSB(itemId) == link->getLinkID())
         return link;
      link = static_cast<AlteredGfxLink*>(link->suc());
   }
   return NULL;
}

void
AlteredGfxList::setRemoveLastCoordinate(uint32 nodeId, GfxData* gfx)
{
   mc2dbg2 << "Removing last coord in route segment" << endl;
   AlteredGfxLink* link = getItemInList(REMOVE_UINT32_MSB(nodeId));
   if(link == NULL)
   {
      link = new AlteredGfxLink(REMOVE_UINT32_MSB(nodeId));
      link->into(this);
   }
   bool isNode0 = (nodeId == REMOVE_UINT32_MSB(nodeId));
   int32 newLat, newLon;
   bool newCoord = checkNewCoord(gfx, !isNode0, newLat, newLon);
   if(isNode0){
      link->removeLastCoord();
      if(newCoord)
         link->addLastCoord(newLat, newLon);
   }else{
      link->removeFirstCoord();
      if(newCoord)
         link->addFirstCoord(newLat, newLon);
   }
}

void
AlteredGfxList::setRemoveFirstCoordinate(uint32 nodeId, GfxData* gfx)
{
   mc2dbg2 << "Removing first coord in route segment" << endl;
   AlteredGfxLink* link = getItemInList(REMOVE_UINT32_MSB(nodeId));
   if(link == NULL)
   {
      link = new AlteredGfxLink(REMOVE_UINT32_MSB(nodeId));
      link->into(this);
   }
   bool isNode0 = (nodeId == REMOVE_UINT32_MSB(nodeId));
   int32 newLat, newLon;
   bool newCoord = checkNewCoord(gfx, isNode0, newLat, newLon);
   if(isNode0)
   {
      link->removeFirstCoord();
      if(newCoord)
         link->addFirstCoord(newLat, newLon);
   }else{
      link->removeLastCoord();   
      if(newCoord)
         link->addLastCoord(newLat, newLon);
   }
}

bool
AlteredGfxList::removeFirstCoord(uint32 nodeId, int32 &newLat, int32 &newLon)
{
   AlteredGfxLink* link = getItemInList(REMOVE_UINT32_MSB(nodeId));
   newLat = MAX_INT32;
   newLon = MAX_INT32;
   if(link != NULL){
      bool isNode0 = (nodeId == REMOVE_UINT32_MSB(nodeId));
      if(isNode0){
         if(link->getRemoveFirst()){
            link->getFirstCoord(newLat, newLon);
            return true;
         }
      } else {
         if(link->getRemoveLast()){
            link->getLastCoord(newLat, newLon);
            return true;
         }
      }
   }
   return false;
}

bool
AlteredGfxList::removeLastCoord(uint32 nodeId, int32 &newLat, int32 &newLon)
{
   AlteredGfxLink* link = getItemInList(REMOVE_UINT32_MSB(nodeId));
   newLat = MAX_INT32;
   newLon = MAX_INT32;
   if(link != NULL){
      bool isNode0 = (nodeId == REMOVE_UINT32_MSB(nodeId));
      if(isNode0){
         if(link->getRemoveLast()){
            link->getLastCoord(newLat, newLon);
            return true;
         }
      } else {
         if(link->getRemoveFirst()){
            link->getFirstCoord(newLat, newLon);
            return true;
         }
      }
   }
   return false;
}


bool
AlteredGfxList::checkNewCoord(GfxData* gfx, bool begining,
                              int32 &newLat, int32 &newLon)
{
   newLat = MAX_INT32;
   newLon = MAX_INT32;
   int32 lat1, lon1, lat2, lon2;
   
   DEBUG1( 
      if (gfx->getNbrPolygons() != 1) {
         mc2log << warn << here << " Only using first polygon!" << endl;
      }
   );
   
   if(begining){
      lat1 = gfx->getLat(0,0);
      lon1 = gfx->getLon(0,0);
      lat2 = gfx->getLat(0,1);
      lon2 = gfx->getLon(0,1);
   } else {
      uint32 points = gfx->getNbrCoordinates(0);
      lat1 = gfx->getLat(0,points-1);
      lon1 = gfx->getLon(0,points-1);
      lat2 = gfx->getLat(0,points-2);
      lon2 = gfx->getLon(0,points-2);
   }
   if(!GfxUtility::getPointOnLine(lat1, lon1, lat2, lon2, 5, newLat, newLon))
   {
      newLat = MAX_INT32;
      newLon = MAX_INT32;
      return false;
   }
   mc2dbg4 << "       coord at : " << lat1   << ", " <<lon1<< endl;
   mc2dbg4 << "Adding coord at : " << newLat << ", " <<newLon << endl;
   mc2dbg4 << "       coord at : " << lat2   << ", " <<lon2<< endl;
   return true;
}


