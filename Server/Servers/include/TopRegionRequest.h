/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TOPREGIONREQUEST_H
#define TOPREGIONREQUEST_H

#include "config.h"
#include "Request.h"
#include "TopRegionMatch.h"

/**
 * Retrieves the top regions of the world.
 *
 */
class TopRegionRequest : public RequestWithStatus {
   public:
      /**
       *   Creates a TopRegionRequest.
       *
       *   @param reqID The request ID.
       */
      TopRegionRequest( uint32 reqID );

      /**
       *   Creates a TopRegionRequest.
       *
       *   @param parent parent request
       */
      TopRegionRequest( Request* parent );

      /**
       * Destructor.
       */
      ~TopRegionRequest();


      /**
       * Process the data returned by a module.
       * @param pack Packet container with the packet that was
       *             processed by the modules.
       */
      virtual void processPacket( PacketContainer* pack );


      /**
       * Get the number of top regions.
       * 
       * @return The number of top regions.
       */
      uint32 getNbrTopRegions() const;


      /**
       *   Returns a top region with type country that contains
       *   the map with the supplied ID or NULL.
       *   @param mapID Map to look for.
       *   @return Country containing the map or NULL if not found.
       */
      const TopRegionMatch* getCountryForMapID( uint32 mapID ) const;
      
      /**
       * Get top region at index i.
       *
       * @param i The index of the TopRegionMatch to get.
       * @return The top region at index i, or NULL if index outofbounds.
       */
      const TopRegionMatch* getTopRegion( uint32 i ) const;


      /**
       * Get top region with an specific id.
       *
       * @param id The id of the top region to get.
       * @return The top region with id, or NULL if no such id.
       */
      const TopRegionMatch* getTopRegionWithID( uint32 id ) const;

      /**
       * Get top region that has a bounding box that overlaps with
       * the specified bounding box. The first found top region will be
       * returned.
       *
       * @param bbox MC2BoundingBox to check for overlap
       * @return The top region with overlap, or NULL if no overlap found
       */
      const TopRegionMatch* getTopRegionWithBBoxOverlap( 
                                            const MC2BoundingBox& bbox ) const;

      /**
       * Get top region that has a bounding box that contains the specified
       * coordinate. The first found top region will be returned.
       *
       * @param coord The coordinate to use
       * @return The top region containing the coordinate, or NULL if no overlap found
       */
      const TopRegionMatch* getTopRegionByCoordinate(
                                              const MC2Coordinate& coord) const;

      /**
       *   Finds the match by (currently) english name onyl.
       *   @return NULL if not found.
       */
      const TopRegionMatch* getTopRegionByName( const char* name ) const;

      /**
       * Returns StringTable::OK if request was successful.
       * @return StringTable::OK if request was succesful.
       *         StringTable::TIMEOUT_ERROR if a timeout occured
       *         StringTable::INTERNAL_SERVER_ERROR on unexpected
       *         state or packet type.
       */
      StringTable::stringCode getStatus() const;


      /**
       * Get the Whole ItemID tree.
       *
       * @return THe Whole ItemID tree.
       */
      const ItemIDTree& getWholeItemIDTree() const;

      /**
       *    Get the whole match vector.
       */
      const TopRegionMatchesVector& getTopRegionVector() const;

      /**
       * Add a top region.
       *
       * @param match The match to add, this request now owns the match.
       */
      void addTopRegion( TopRegionMatch* match );

   private:

      /// common code for the constructors
      void init();

      /**
       * The states of this request.
       */
      enum {
         /// Sending and receiving top region request.
         TOP_REGION,

         /// Top regions received ok.
         DONE,

         /**   
          * Something went wrong when processing any part of the 
          * task for this request.
          */
         ERROR,
      } m_state;

      /// "sub" state, here for handling of more than one set of map modules
      uint32 m_repliesToReceive;

      TopRegionMatchesVector m_matches;


      // The whole ItemIDTree.
      ItemIDTree m_wholeTree;
};


#endif // TOPREGIONREQUEST_H

