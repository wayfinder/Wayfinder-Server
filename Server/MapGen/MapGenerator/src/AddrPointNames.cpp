/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "AddrPointNames.h"

#include <fstream>
#include "GDFRef.h"
#include "CharEncoding.h"
#include "NationalProperties.h"
#include "StringTable.h"
#include "STLStringUtility.h"
#include "OldGenericMap.h"


#include "MapGenUtil.h"
#include <vector>

AddrPointNames::~AddrPointNames(){
   // Empty for now.
}

AddrPointNames::AddrPointNames(MC2String fileName){
   m_itMapID = MAX_UINT32;
   
   // Load the file.
   mc2dbg << "AddrPointNames Loading from " << fileName << endl;
   vector< vector <MC2String> > tmpTxtTbl;
   bool result = 
      MapGenUtil::loadTxtFileTable( fileName, 
                                    4,  // Number of columns
                                    ";", // Column separator
                                    tmpTxtTbl ); 
   if ( ! result ){
      mc2log << error << "AddrPointNames failed loading file: " << fileName 
             << ". exits!"
             << endl;
      exit(1);
   }

   // Parse the file.
   for ( uint32 i=0; i<tmpTxtTbl.size(); i++){
      MC2String addrPointID = tmpTxtTbl[i][0];

      addrPoint_t addrPoint;
      addrPoint.coord = 
         MC2Coordinate(STLStringUtility::strtol(tmpTxtTbl[i][1]),
                       STLStringUtility::strtol(tmpTxtTbl[i][2]));
      addrPoint.name = tmpTxtTbl[i][3];
      addrPoint.closestDist = MAX_UINT32; // Default value
      addrPoint.closestDistMapID = MAX_UINT32; // Default value
      m_addrPointsByID.insert(make_pair(addrPointID, addrPoint));
      mc2dbg8 << "Loaded " << addrPointID << ": " << addrPoint << endl;
   }
   mc2dbg << "Size of m_addrPointsByID " << m_addrPointsByID.size() << endl;
   if ( m_addrPointsByID.size() == 0 ){
      mc2log << error << "AddrPointNames empty file: " << fileName 
             << ". exits!"
             << endl;
      exit(1);
   }
   m_addrPointIt = m_addrPointsByID.begin();
} // constructor AddrPointNames

void
AddrPointNames::storeSsiDistances(OldGenericMap* theMap){
   mc2dbg << "Checking distances to SSIs of map 0x" << hex 
          << theMap->getMapID() << dec << endl;

   // Use for closest distance calculation.
   set< ItemTypes::itemType > allowedTypes;
   allowedTypes.insert(ItemTypes::streetSegmentItem);
   uint64 dist;

   MC2BoundingBox mapBBox;
   theMap->getMapBoundingBox(mapBBox);
   mc2dbg8 << "Map Box: " << mapBBox << endl;

   uint32 inBBox = 0;
   AddrPointNames::idAndAddrPoint_t idAndAddrPt = getFirstAddrPoint();
   while (idAndAddrPt.first != MC2String("")){
      // Only check coordinates within the map bounding box.
      if (mapBBox.inside(idAndAddrPt.second.coord.lat,
                         idAndAddrPt.second.coord.lon) ){
         inBBox++;
         theMap->getClosestItemID(idAndAddrPt.second.coord,
                                  dist, 
                                  allowedTypes);
         setClosestSSIMapOfAddrPoint(idAndAddrPt.first, // ID
                                     theMap->getMapID(),
                                     dist);
      }
      idAndAddrPt = getNextAddrPoint();
   }
   mc2dbg << "Number of address points in map bbox of map 0x" << hex 
          << theMap->getMapID() << dec << ": " << inBBox << endl;
}

AddrPointNames::idAndAddrPoint_t 
AddrPointNames::getFirstAddrPoint(){
   m_addrPointIt = m_addrPointsByID.begin();
   return getNextAddrPoint();
}

AddrPointNames::idAndAddrPoint_t 
AddrPointNames::getNextAddrPoint(){
   idAndAddrPoint_t result;
   if ( m_addrPointIt != m_addrPointsByID.end() ){
      result = *m_addrPointIt;
      ++m_addrPointIt;
   }
   return result;

} // getNextAddrPoint


bool 
AddrPointNames::setClosestSSIMapOfAddrPoint(MC2String addrPointID,
                                            uint32 mapID, 
                                            uint32 distance)
{
   bool result = true;
   addrPointByID_t::iterator it = m_addrPointsByID.find(addrPointID);
   if (it ==  m_addrPointsByID.end()){
      mc2log << error << "AddrPointNames address point ID " << addrPointID
             << " missing in this container."
             << endl;
      result = false;
   }
   else {
      if ( distance < it->second.closestDist ){
         it->second.closestDist = distance;
         it->second.closestDistMapID = mapID;
      }
   }
   return result;
} // setClosestSSIMapOfAddrPoint


uint32
AddrPointNames::addAddrPointNamesToMap(OldGenericMap* theMap){
   // Use for closest distance calculation.
   set< ItemTypes::itemType > allowedTypes;
   allowedTypes.insert(ItemTypes::streetSegmentItem);
   uint64 dist;


   uint32 addedNames = 0;
   AddrPointNames::addrPoint_t addrPoint = 
      getNextAddrPointOfMap(theMap->getMapID());
   while (addrPoint.coord.isValid()){
      mc2dbg1 << "AddrPt: " << addrPoint << endl;
      uint32 itemID = theMap->getClosestItemID(addrPoint.coord,
                                               dist, 
                                               allowedTypes);
      // Maximun 500 m from road.
      uint64 maxDistForAdd = 
         static_cast<int64>(GfxConstants::METER_TO_MC2SCALE*500);
      maxDistForAdd = maxDistForAdd*maxDistForAdd; // Square
      if ( dist < maxDistForAdd ){
         // FIXME: this is hardocded to english.
         // If you want another language for your alternative name change it,
         // perhaps via properties in NationalProperties or using the
         // native language of the mcm map.
         theMap->addNameToItem(itemID, 
                               addrPoint.name.c_str(), 
                               LangTypes::english,
                               ItemTypes::alternativeName);
         mc2dbg1 << "Added name: " << addrPoint.name << " to item: 0x" 
                 << hex << itemID << dec << endl;
         addedNames++;
      }
      addrPoint = getNextAddrPointOfMap(theMap->getMapID());
   }
   return addedNames;
}


AddrPointNames::addrPoint_t
AddrPointNames::getNextAddrPointOfMap(uint32 mapID){

   // Fill in m_addrPointsByMapID if it is empty.
   if ( m_addrPointsByMapID.size() == 0 ){
      mc2dbg << "AddrPointNames Initiating m_addrPointsByMapID" << endl;
      for ( addrPointByID_t::const_iterator addrPointIt = 
               m_addrPointsByID.begin();
            addrPointIt != m_addrPointsByID.end(); ++addrPointIt){
         if ( addrPointIt->second.closestDist == MAX_UINT32 ){
            mc2log << error << "AddrPointNames no closest dist set for "
                   << "addres point ID " << addrPointIt->first 
                   << ". Exits!"
                   << endl;
            exit(1);
         }
         const addrPoint_t* addrPoint = &addrPointIt->second;
         m_addrPointsByMapID.insert(make_pair(addrPoint->closestDistMapID, 
                                              addrPoint));
         mc2dbg8 << "Building m_addrPointsByMapID: " << *addrPoint << endl;
      }
      mc2dbg << "Size of m_addrPointsByMapID " << m_addrPointsByMapID.size() 
             << endl;
   }
   
   // Reset the iterator if needed.
   if ( mapID != m_itMapID ){
      m_mapDependIt = m_addrPointsByMapID.lower_bound(mapID);
      m_itMapID = mapID;
   }
   addrPoint_t result; // Defaults with invalid coordinate.
   if (m_mapDependIt != m_addrPointsByMapID.upper_bound(mapID) ){
      result = *(m_mapDependIt->second);
      ++m_mapDependIt;
   }
   return result;
} // getNextAddrPointOfMap

ostream& operator<< ( ostream& stream, const AddrPointNames::addrPoint_t& addrPoint ){
   return stream << addrPoint.name << " " << addrPoint.coord
                 << " dist:" << addrPoint.closestDist << " mapID:"
                 << addrPoint.closestDistMapID;
}
                                     
