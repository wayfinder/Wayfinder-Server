/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ADDRPOINTNAMES_H
#define ADDRPOINTNAMES_H


#include "config.h"
#include <map>

#include "MC2String.h"
#include "MC2Coordinate.h"

class OldGenericMap;
// class CharEncoding; Not needed right now. The data is in UTF-8

/**
  *   AddrPointNames is a container that stores the data of a address point
  *   file, and have methods for adding these names to SSIs.
  *
  *   This class works in three steps.
  *   1) The file with address point information is read from disc and stored
  *      the container.
  *   2) Call storeSsiDistances with all candidate maps as argument.
  *   3) Call addAddrPointNamesToMap with all maps used when calling 
  *      storeSsiDistances.
  *
  */
class AddrPointNames
{
 public:
   
   /**
    * Constructor, loads the address point file.
    */
   AddrPointNames(MC2String fileName);
   
   /**
    * Destructor.
    */
   ~AddrPointNames();
      
   /// Stores data of an address point.
   struct addrPoint_t {
      /// Coordinate of this address point.
      MC2Coordinate coord;
      /// Name of this address point.
      MC2String name;

      /// Closest distance to an SSI in the map with closestDistMapID
      uint32 closestDist;
      /// The ID of the map where the closest SSI has been found.
      uint32 closestDistMapID;

      // Defined in the cpp file.
      friend ostream& operator<< ( ostream& stream, 
                                   const addrPoint_t& addrPoint );
   };

   /**
    * Finds distance to SSIs within the map from all address within the 
    * bounding box of theMap, if the distance is smaller than the distance
    * already stored. Call this method with all maps being candidates for 
    * address point name addition.
    */
   void storeSsiDistances(OldGenericMap* theMap);

   /**
    * When storeSsiDistances has been called with all candidate maps, call this
    * method with the same maps to add the names to SSIs of the right map.
    *
    * @return Returns number of names added to SSIs.
    */
   uint32 addAddrPointNamesToMap(OldGenericMap* theMap);



 protected:
   /**
    * @return Returns empty MC2String in first if no more address ponts exists.
    */
   typedef pair<MC2String, addrPoint_t> idAndAddrPoint_t;
   idAndAddrPoint_t getFirstAddrPoint();
   idAndAddrPoint_t getNextAddrPoint();
   
   /**
    * @return False if the address point ID was not found in this container.
    */
   bool setClosestSSIMapOfAddrPoint(MC2String addrPointID,
                                    uint32 mapID, 
                                    uint32 distance);
   /**
    * @return Returns an addrPonit_t struct. The coord value of this one is 
    *         invalid if this no more address point of this map exists.
    */
   addrPoint_t getNextAddrPointOfMap(uint32 mapID);   


   typedef map<MC2String, addrPoint_t> addrPointByID_t;
   /** Used for determining which map an address point's names should be added 
    *  to.
    */
   addrPointByID_t m_addrPointsByID;
   /// Iterator of m_addrPointsByID
   addrPointByID_t::const_iterator m_addrPointIt;
   

   typedef multimap<uint32, const addrPoint_t*> addrPointsByMapID_t;
   /// Used to organize the address points by map.
   addrPointsByMapID_t m_addrPointsByMapID;
   /// Iterator of m_addrPointsByMapID.
   addrPointsByMapID_t::const_iterator m_mapDependIt;
   /// The map of which range the iterator m_mapDependIt last was used.
   uint32 m_itMapID;
   
}; // AddrPointNames


#endif // ADDRPOINTNAMES_H.
