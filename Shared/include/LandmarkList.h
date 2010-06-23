/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef LANDMARKLIST_H
#define LANDMARKLIST_H

#include "config.h"
#include "CCSet.h"
#include "SearchTypes.h"
#include "ItemTypes.h"
#include "StringUtility.h" 
// Replace StringUtility.h with GenericMap.h when moved to ServersShared

// Forward declaration
class LandmarkHead;

/**
 *    Describes one landmark. Used when expanding the route.
 *
 */
class LandmarkLink : public Link {
   friend class LandmarkHead;

   public:

      /**
        *    Create a new landmark in the route.
        *    @param description   The landmarkdescription for this landmark,
        *                         e.g. from the lmTable in a map.
        *    @param dist          How far on a route-item is the landmark 
        *                         located. Intitate to m_dist of the routeitem.
        *    @param distUpdated   Default false, set to true when the 
        *                         route-item this landmark belongs to has 
        *                         been concatenated with another routeitem.
        *    @param name          The name of this landmark.
        */
      LandmarkLink(ItemTypes::lmdescription_t description, int32 dist,
            bool distUpdated = false, const char* name = "");

      /**
        *   Delete this object and release the memory allocated here.
        */
      virtual ~LandmarkLink();

      /**
        *   Print information about this landmark to stdout.
        */
      void dump();

      /**
        *   Creates a description of this landmark, text formatted, 
        *   to be used in the route description.
        *   Examples: "After 15 km pass Landskrona" or
        *   "Then turn right after Allhelgonakyrkan on your left side".
        *   @param descProps  Route description properties (bitfield) that
        *                     that indicates which information
        *                     that should be included in the route
        *                     decription.
        *   @param buf        Preallocated buffer which will be filled
        *                     with the description.
        *   @param maxLength  The length of buf.
        *   @param nbrBytesWritten  The number of bytes written into buf.
        *   @param turncode   The turn, which this landmark is associated 
        *                     with.
        *   @return True if the route description did fit in the
        *           buffer, false otherwise.
        */
      bool getRouteDescription( uint32 descProps,
                                char* buf,
                                uint32 maxLength,
                                uint32& nbrBytesWritten,
                                StringTable::stringCode turncode 
                                = StringTable::NOSTRING);

      /**
        *   Set the name of this landmark.
        */
      inline void setLMName(const char* name);

      /**
        *   Get the name of this landmark.
        */
      inline const char* getLMName() const;

      /**
        *   Set whether this landmark is at the end of the route link or not.
        */
      inline void setEndLM(bool endLM);

      /**
        *   Find out if this landmark is at the end of the route link.
        */
      inline bool isEndLM() const;
      
      /** 
        *   Struct with info about this landmark.
        */
      ItemTypes::lmdescription_t m_lmdesc;

      /// How far on the route-item is this landmark located.
      int32 m_lmDist;

      /**
        *   States whether this landmark's m_lmDist has been updated or not 
        *   during use in the ExpandRouteProcessor.
        *   If the route-item to which this landmark belongs is 
        *   concatenated with another route-item, the landmark's m_lmDist 
        *   is also updated. If the m_lmDist has been updated it should 
        *   _not_ be changed if more route-items are added after the 
        *   landmarks's route-item, but should if more route-items are 
        *   added before the lm's route-item.
        */
      bool m_lmDistUpdated;

   private:

      /**
        *   The name of the landmark.
        */
      char* m_lmName;

      /**
        *   Is this landmark an endLM, i.e. at the end of a routelink.
        */
      bool m_endLM;


};

inline void
LandmarkLink::setLMName(const char* name)
{
   mc2dbg8 << "updating m_lmName from \"" << m_lmName << "\" to \"" << name << "\"" << endl;
   delete m_lmName;
   m_lmName = StringUtility::newStrDup(name);
}

inline const char*
LandmarkLink::getLMName() const
{
   return m_lmName;
}

inline void
LandmarkLink::setEndLM(bool endLM)
{
   m_endLM = endLM;
}

inline bool
LandmarkLink::isEndLM() const
{
   return m_endLM;
}

/**
 *    The list that have the landmark-items (objects of the LandmarkLink
 *    class).
 *
 */
class LandmarkHead : public Head {
   public:
      /**
       *    Default constructor. Does not do anything except calling
       *    the constructor in the superclass (Head).
       */
      LandmarkHead();
   
      /**
       *    Delete memory allocated here.
       */
      virtual ~LandmarkHead();

      /**
       *    Print info about the landmark-items in the list.
       *    @return  The number of items in the list. A negative value
       *             is returned upon error.
       */
      int dump();
      
};


#endif

