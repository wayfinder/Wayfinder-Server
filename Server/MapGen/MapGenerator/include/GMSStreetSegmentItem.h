/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GMSSTREETSEGMENTITEM_H
#define GMSSTREETSEGMENTITEM_H

#include "DataBuffer.h"
#include "GMSNode.h"
#include "GMSGfxData.h"
#include <fstream>
#include "OldStreetSegmentItem.h"
#include "GMSItem.h"

#include "OldGenericMap.h"

#include "StringUtility.h"
#include <vector>
#include <stdlib.h>

#include "AllocatorTemplate.h"



class FirstInPairFind {
   public:
      FirstInPairFind(const char* x) { m_x = x; };
      bool operator()( pair<const char*, const char*> y) const {
         return StringUtility::strcmp(m_x, y.first) == 0;
      }
   private:
      const char* m_x;
};


/**
  *   GenerateMapServer street segment item. Streetsegment item
  *   with extra features needed when creating the map.
  *
  */
class GMSStreetSegmentItem :  public OldStreetSegmentItem, 
                              public GMSItem,
                              public VectorElement {
   public:
   
      /**
        *   Create an empty OldStreetSegmentItem.
        */
      GMSStreetSegmentItem();

      /**
        *   Create an empty OldStreetSegmentItem.
        *   @param map the map
        */
      GMSStreetSegmentItem( OldGenericMap *map );

      /**
        *   Create a Street Segment item with attributes from 
        *   another segment.
        *   @param   map   The map.
        *   @param   ssi   The other segment.
        */
      GMSStreetSegmentItem( OldGenericMap* map, GMSStreetSegmentItem* ssi);

      /**
        *   Create a OldStreetSegmentItem with a specified ID.
        *   @param   id    The ID of this item.
        *   @param   map   The map
        */
      GMSStreetSegmentItem( OldGenericMap *map, uint32 id,
                            GMSItem::map_t mapType = GDF);
      
      /**
        *   Constructor.
        */
	   GMSStreetSegmentItem(uint32 id);

      /**
        *   Constructor.
        */
	   GMSStreetSegmentItem(uint32 id, OldGenericMap* map);

      /**
        *   Release allocated memory
        */
      virtual ~GMSStreetSegmentItem();

      /**
        *   Create a gms street segment item (including nodes) from mid/mif 
        *   files. If the mid file does not have a value for any of the gms 
        *   ssi's member variables, the default values are used (i.e. the 
        *   values assigned to a street segment item in its constructor).
        *   @param midFile  The attribute file, from which to read values
        *                   for the different member variables for this gms 
        *                   street segment item.
        *   @param readRestOfLine   If the rest of the mid line (eol) should 
        *                           be read or not, default true.
        */
      bool createFromMidMif(ifstream& midFile, bool readRestOfLine = true);


      /**
        *   Get one of the nodes.
        *   @param   i  The node to return. Valid values are 0 for
        *               noe 0 and 1 for node 1.
        *   @return  Pointer to the node (GMSNode*) asked for (0 or 1).
        */
      inline GMSNode* getNode(int i);
      
      /**
        *   
        */
      inline void setWidth(uint8 w);

      /**
        *   
        */
      inline void setCondition(byte cond);

      /**
       *    Set the temporary functional road class. Will be used for
       *    setting estimated speed limit.
       *    @param   tempRoadClass  The temporary road class.
       */
      inline void setTempRoadClass(uint32 tempRoadClass);

      /**
       *    Get the temporary functional road class. Will be used for
       *    setting estimated speed limit.
       *    @return   tempRoadClass  The temporary road class.
       */
      inline uint32 getTempRoadClass() const;
      
      /**
       *    Set the temporary net2 class. Will be used for
       *    setting estimated speed limit.
       *    @param   net2Class  The temporary net2 class.
       */
      inline void setTempNet2Class(uint32 net2Class);

      /**
       *    Get the temporary net2 class. Will be used for
       *    setting estimated speed limit.
       *    @return   tempNet2Class  The temporary net2 class.
       */
      inline uint32 getTempNet2Class() const;



      /**
       * $name Get and set the temporary special restriction attribute. Used 
       *       when interpretting attributes influencing the entry 
       *       restrictions.
       */
      //@{
      inline void setTempSpecRestriction(
         ItemTypes::entryrestriction_t restriction );
      inline ItemTypes::entryrestriction_t getTempSpecRestriction() const;
      //@}

      /**
       * $name Get and set the stubble attribute.
       */
      //@{
      inline void setTempStubble(bool stubble);
      inline bool getTempStubble() const;
      //@}

      /**
       * $name Get and set the back road attribute.
       */
      //@{
      inline void setTempBackroad(uint32 backroad);
      inline uint32 getTempBackroad() const;
      //@}

      /**
       * $name Get and set the construction status.
       */
      //@{
      inline void setTempConstructionStatus(uint32 status);
      inline uint32 getTempConstructionStatus() const;
      //@}

      /**
       * $name Get and set the no trough traffic attribute.
       */
      //@{
      inline void setTempNoThroughTraffic(bool noThroughTraffic);
      inline bool getTempNoThroughTraffic() const;
      //@}

      /**
       * $name Get and set the rough road attribute.
       */
      //@{
      inline void setTempRoughRoad(bool roughRoad);
      inline bool getTempRoughRoad() const;
      //@}


      /**
       *    Set the temporary form of way. Will be used for
       *    setting estimated speed limit.
       *    @param   tempFormOfWay  The temporary form of way.
       */
      inline void setTempFormOfWay(uint32 tempFormOfWay);

      /**
       *    Get the temporary form of way. Will be used for
       *    setting estimated speed limit.
       *    @return   tempFormOfWay  The temporary form of way.
       */
      inline uint32 getTempFormOfWay() const;
      
      /**
        */
      uint32 belongsToStreet;

      /**
        */
      int32 indexInStreet;

      /**
        */
      uint32 belongsToBranch;

      /**
        */
      bool  backwards;

      /**
        *   Pointer to the OldMap.
        */
      OldGenericMap *m_map;
      


      /**
       *    Will not add the same zipCode more than once, but will
       *    assume that the zipArea not is significant (will not be used
       *    in comparisons and might be null). The strings are copied,
       *    so the caller are free to delete the strings sent as parameters.
       *    @param zipCode The zip code to add to this street.
       *    @param zipArea The postal area of the zip code. Might be 
       *                   null.
       */
      void addTmpZipCode(const char* zipCode, const char* zipArea) {
         MC2_ASSERT(zipCode != NULL);
         vector< pair<const char*, const char*> >::const_iterator it = 
             find_if(m_zips.begin(), m_zips.end(), FirstInPairFind(zipCode));
         if (it == m_zips.end()) {
            const char* zc = StringUtility::newStrDup(zipCode);
            const char* za = NULL;
            if (zipArea != NULL)
               za = StringUtility::newStrDup(zipArea);
            m_zips.push_back(make_pair(zc, za));
         }
      }

      uint32 getNbrTmpZipCodes() {
         return m_zips.size();
      }

      const char* getTmpZipCode(uint32 zipNbr) {
         MC2_ASSERT(zipNbr < m_zips.size());
         return m_zips[zipNbr].first;
      }

   protected : 
      /**
        *
        */
      virtual OldNode* createNewNode(DataBuffer* dataBuffer, uint32 nodeID) {
         mc2dbg8 << here << "Creating new GMSNode" << endl;
         return (new GMSNode(dataBuffer, nodeID));
      }

      virtual OldNode* getNewNode(OldGenericMap* theMap) {
         mc2dbg8 << here << "Creating new GMSNode" << endl;
         return (static_cast<MC2Allocator<GMSNode>*>
                            (theMap->m_nodeAllocator)->getNextObject());
      }

      /**
        *
        */
      virtual GfxData* createNewGfxData(DataBuffer* dataBuffer) {
         mc2dbg << "Creating new GMSGfxData" << endl;
         return GMSGfxData::createNewGfxData(dataBuffer, NULL);
      }

   private:
      /**
        *   Copy the attributes from another ssi to this ssi.
        *   @param   ssi   The other ssi.
        */
      void copyAttributes(GMSStreetSegmentItem* ssi);


      /**
        *   Initiate the membervariables in this object.
        */
      void initMembers();

      /**
       *    Array with pairs of { zip_code, zip_area }, used for 
       *    temporary storage during map generation. Before the map is 
       *    saved to disc, this data must collected and stored in 
       *    OldZipCodeItems and OldZipAreaItems.
       */
      vector< pair<const char*, const char*> > m_zips;

      /**
       *    Temporary variable for functional road class. Will
       *    be used for setting speed limits (TA)
       */
      uint32 m_tempRoadClass;
      
      /**
       *    Temporary variable for net2 class. Will
       *    be used for setting speed limits (TA)
       */
      uint32 m_tempNet2Class;

      /**
       *    Temporary variable for form of way. Will
       *    be used for setting speed limits (TA)
       */
      uint32 m_tempFormOfWay;


      /**
       *    Set appropriate value if the special restriction attribute has 
       *    been found for this item.
       */
      ItemTypes::entryrestriction_t m_tempSpecRestriction;

      /**
       *    Set to true if the item is a stubble.
       */
      bool m_tempStubble;

      /**
       *    Temp variable for back road attribute. Will be used
       *    to identify inter-block passages IPM in Moscow.
       */
      uint32 m_tempBackroad;

      /**
       *    Temp variable for construction status.
       */
      uint32 m_tempConstructionStatus;

      /**
       *    Temp variable, set to true if the item has No Trrough Traffic.
       */
      bool m_tempNoThroughTraffic;

      /**
       *    Temp variable, set to true if the item is a Rough road.
       */
      bool m_tempRoughRoad;

};


// ==================================================================
//                  Implementation of (some of the) inlined methods =


inline GMSNode* 
GMSStreetSegmentItem::getNode(int i)
{
   return (static_cast<GMSNode*> (OldRouteableItem::getNode(i)));
}

void
GMSStreetSegmentItem::setWidth(uint8 w)
{
   m_width = w;
}

void
GMSStreetSegmentItem::setCondition(byte cond)
{
   m_condition = cond;
}

inline void
GMSStreetSegmentItem::setTempRoadClass(uint32 tempRoadClass)
{
   m_tempRoadClass = tempRoadClass;
}

inline uint32 
GMSStreetSegmentItem::getTempRoadClass() const
{
   return m_tempRoadClass;
}

inline void
GMSStreetSegmentItem::setTempNet2Class(uint32 tempNet2Class)
{
   m_tempNet2Class = tempNet2Class;
}

inline uint32 
GMSStreetSegmentItem::getTempNet2Class() const
{
   return m_tempNet2Class;
}

inline void
GMSStreetSegmentItem::setTempFormOfWay(uint32 tempFormOfWay)
{
   m_tempFormOfWay = tempFormOfWay;
}

inline uint32 
GMSStreetSegmentItem::getTempFormOfWay() const
{
   return m_tempFormOfWay;
}


inline void
GMSStreetSegmentItem::setTempSpecRestriction(
   ItemTypes::entryrestriction_t restriction)
{
   m_tempSpecRestriction = restriction;
}

inline ItemTypes::entryrestriction_t 
GMSStreetSegmentItem::getTempSpecRestriction() const
{
   return m_tempSpecRestriction;
}

inline void
GMSStreetSegmentItem::setTempStubble(bool stubble)
{
   m_tempStubble = stubble;
}

inline bool 
GMSStreetSegmentItem::getTempStubble() const
{
   return m_tempStubble;
}

inline void 
GMSStreetSegmentItem::setTempBackroad(uint32 backroad)
{
   m_tempBackroad = backroad;
}
inline uint32
GMSStreetSegmentItem::getTempBackroad() const
{
   return m_tempBackroad;
}

inline void
GMSStreetSegmentItem::setTempConstructionStatus(uint32 status)
{
   m_tempConstructionStatus = status;
}
inline uint32
GMSStreetSegmentItem::getTempConstructionStatus() const
{
   return m_tempConstructionStatus;
}

inline void
GMSStreetSegmentItem::setTempNoThroughTraffic(bool noThroughTraffic)
{
   m_tempNoThroughTraffic = noThroughTraffic;
}

inline bool 
GMSStreetSegmentItem::getTempNoThroughTraffic() const
{
   return m_tempNoThroughTraffic;
}

inline void
GMSStreetSegmentItem::setTempRoughRoad(bool roughRoad)
{
   m_tempRoughRoad = roughRoad;
}

inline bool 
GMSStreetSegmentItem::getTempRoughRoad() const
{
   return m_tempRoughRoad;
}



#endif
