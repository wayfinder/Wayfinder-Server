/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPNOTICE_H
#define MAPNOTICE_H


#include "config.h"
#include "VectorElement.h"

/**
 *    MapNotice.
 *
 */
class MapNotice : public VectorElement {
   public :

      MapNotice();

      MapNotice(uint32 mapID);
      
      virtual ~MapNotice();

      /**
        *   The operators needed for sorting and searching.
        */
      //@ {
         /// equal
         bool operator == (const VectorElement& elm) const {
            return(m_mapID == (dynamic_cast<const MapNotice*>(&elm))->m_mapID);
         }
         /// not equal
         bool operator != (const VectorElement& elm) const {
            return(m_mapID != (dynamic_cast<const MapNotice*>(&elm))->m_mapID);
         }

         /// greater
         bool operator > (const VectorElement& elm) const {
            return (int32(m_mapID) >
                    int32((dynamic_cast<const MapNotice*>(&elm))->m_mapID));
         }

         /// less
         bool operator < (const VectorElement& elm) const {
            return(int32(m_mapID) <
                   int32((dynamic_cast<const MapNotice*>(&elm))->m_mapID));
         }
      //@ }

      /**
        *   Get the Id of the map (inlined).
        *   @return mapID
        */
      uint32 getMapID() const
         {return (m_mapID);}

      /**
        *   Get the number of requests on the map (inlined).
        *   @return The number of requests on the map.
        */
      uint32 getNbrReq() const
         {return (m_nbrOfRequests);}

      /**
        *   Get the number of modules with the map (inlined).
        *   @return The number of modules with the map.
        */      
      uint32 getNbrMod() const
         {return (m_nbrOfModules);}

      /**
        *   Get the number of modules loading the map (inlined).
        *   @return The number of modules loading the map (0 or 1).
        */
      uint32 getNbrLoading() const
         {return (m_nbrLoading);}

      /**
       *  Add a request to the map (inlined).
       */ 
      void addRequest();
     
      /**
       *  Reset the number of requests (inlined).
       */ 
      void resetRequests()
         { m_nbrOfRequests = 0;}

      /**
       *  Halve the number of requests (inlined).
       */
      void halveRequests()
         { m_nbrOfRequests = m_nbrOfRequests/2;}

      /**
       *  Add a module to the number of modules with the map loaded.
       */
      void addModules()
         { m_nbrOfModules++;}

      /**
       *  Remove a module from the number of modules with the map loaded.
       */
      void reduceModules()
         { m_nbrOfModules--;}

      /**
       *  Set that no modules have the map loaded.
       */ 
      void resetModules()
         { m_nbrOfModules = 0;}

      /**
       *  Specify the number of modules that are loading this map.
       *  @param nbr The number of modules.
       */
      void setLoading(uint32 nbr)
         { m_nbrLoading = nbr;}
      
   protected:
      /**
        *   MapID of this map.
        */
      uint32 m_mapID;

      /// The number of requests on this map.
      uint32 m_nbrOfRequests;

      /// The number of modules with this map loaded.
      uint32 m_nbrOfModules;

      /// The number of modules loading this map.
      uint32 m_nbrLoading;
};

#endif

