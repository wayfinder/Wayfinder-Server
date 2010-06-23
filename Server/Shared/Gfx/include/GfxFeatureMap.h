/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef GFX_FEATURE_MAP_H
#define GFX_FEATURE_MAP_H

#include "config.h"
#include "DataBuffer.h"
#include "MC2BoundingBox.h"
#include "GfxFeature.h"
#include "ScreenSize.h"

/**
 *    Class for holding a GfxFeatureMap.
 *
 */

typedef std::vector < GfxFeature* > GfxFeatureVector;

class GfxFeatureMap {
public:


      /**
       *    Creates an empty map.
       */
      GfxFeatureMap();
      
      /**
       *    Delete all data in the map.
       */
      virtual ~GfxFeatureMap();
      
      /**
       *    Load GfxFeatureMap data from the DataBuffer data.
       *    If reading fails no data is inserted into the map.
       *
       *    @param data DataBuffer with the GfxFeatureMap data to read.
       *    @return True if all data was read and inserted into map,
       *            false if error reading GfxFeatureMap data.
       */
      bool load( DataBuffer* data );


      /**
       * Load GfxFeatureMap header from the DataBuffer data.
       * If reading fails no data is inserted into the map.
       * This method is used when merging maps into a packet without
       * creation feature objects.
       *
       * @param data DataBuffer with the GfxFeatureMap data to read.
       * @return Number of features after header, MAX_UINT32 if error.
       */
      uint32 loadHeader( DataBuffer* data );


      /**
       *    Save GfxFeatureMap data into the DataBuffer data.
       *    If saving fails no partial data should be left in the DataBuffer.
       *
       *    @param data DataBuffer to insert GfxFeatureMap data into, must be
       *                at least getMapSize bytes long.
       *    @return True if all map data was inserted into the DataBuffer,
       *            false if error while saving GfxFeatureMap data.
       */
      bool save( DataBuffer* data ) const;


      /**
       * Writes the header data into data.
       * This method is used when merging maps into a packet without
       * creation feature objects.
       *
       * @param data DataBuffer to insert GfxFeatureMap header into.
       * @param nbrFeatures The number of features to write in header.
       * @return True.
       */
      bool saveHeader( DataBuffer* data, uint32 nbrFeatures ) const;


      /**
       * Save GfxFeatureMap data into the DataBuffer data using the 
       * featureData buffer as feature data.
       * This method is used when merging maps into a packet without
       * creation feature objects.
       *
       * @param data DataBuffer to insert GfxFeatureMap data into, must be
       *             at least getMapSize + featureData->getCurrentOffset 
       *             bytes long.
       * @return True if all map data was inserted into the DataBuffer,
       *         false if error.
       */
      bool save( DataBuffer* data, DataBuffer* featureData, 
                 uint32 nbrFeatures ) const;


      /**
       *    The size of the map when saved into a DataBuffer, is updated
       *    when addFeature or removeFeature is called.
       *
       *    @retrun The number of bytes the map whould take if put into a 
       *            DataBuffer.
       */
      uint32 getMapSize() const;
      
      /**
       */
      inline uint32 getScaleLevel() const;

      /**
       */
      inline void setScaleLevel(uint32 newScale);

      /**
       *    Get the number of Features in the map.
       *    @return The number of features in the map.
       */
      uint32 getNbrFeatures() const;

      /**
       *    Get a feature from the map.
       *
       *    @param index The index of the feature to get.
       *    @return The feature at index index or NULL if >= number
       *            Features.
       */
      const GfxFeature* getFeature( uint32 index ) const;
      
      /**
       *    Add a feature to the map, is added after existing features
       *    in the map.
       *    The map takes over the ownership of the feature and deletes
       *    it with the map or when it is remove with removeFeature.
       * 
       *    @param feature The GfxFeature to add.
       */
      void addFeature( GfxFeature* feature );
      
      /**
       *    Remove and delete feature at index index.
       * 
       *    @param index The index of the feature to remove.
       *    @return True if feature was removed or false if index was
       *            out of range.
       */
      bool removeFeature( uint32 index );
      
      /**
       *    Set boundingbox for map.
       * 
       *    @param   bbox  The boundingbox for map.
       */
      void setMC2BoundingBox(const MC2BoundingBox* bbox);

      /**
       *    Get boundingbox for map.
       *
       *    @param   bbox  Preallocated boundingbox which will be set to
       *                   the boundingbox of the map.
       */
      void getMC2BoundingBox(MC2BoundingBox* bbox);

      /**
       *    Set the width and height of the screen (in pixels).
       * 
       *    @param   
       */
      void setScreenSize(uint16 screenX, uint16 screenY) {
         setScreenSize( ScreenSize( screenX, screenY ) );
      }

      /// Set the size of the screen.
      void setScreenSize( const ScreenSize& size );

      /**
       *    Get the width of the screen (in pixels).
       *
       *    @return The width of the screen.
       */
      inline ScreenSize::ValueType getScreenX() const;
      /// @return The height of the screen in pixels.
      inline ScreenSize::ValueType getScreenY() const;

      /// @return The Size of the screen in pixels.
      inline const ScreenSize& getScreenSize() const;

      /**
       *    Dump information about the map to stdout.
       *    @param verboseLevel Parameter to indicate the amount of output
       *                        from this method.
       */
      void dump(int verboseLevel = 1) const;

      /**
       *    Print statistics about the map.
       */
      void printStatistics(int verboseLevel = 1);
      

      /**
       *    Merge data from another map into this one. No data will be 
       *    copied, so the map given as parameter will be empty after 
       *    this call.
       *    @param otherMap   The map to get the data from, will be empty
       *                      when this method returns.
       *    @param keepOrder  If order should be preseved, default true.
       *    @return The number of features from otherMap that is moved into 
       *            this map. A negative value will be returned upon error.
       */
      int mergeInto(GfxFeatureMap* otherMap,
                    bool keepOrder = true );


      /**
       */
      void recalculateLevelAndSplitRoads();


      /**
       * @name Methods for setting and getting the route settings
       *       of the GfxFeatuteMap.
       */
      //@{
         /**
          * Set transportation type.
          * @param transpType The transportation type.
          */
         inline void setTransportationType(
            ItemTypes::transportation_t transpType );
         
         /**
          * Get transportation type, should only be used if map 
          * contains route_origin feature.
          * @return The transportation type.
          */
         inline ItemTypes::transportation_t getTransportationType() const;

         /**
          * Set starting angle. 0-255 degrees, 0 degrees is north and
          * increasing clockwise.
          * @param startAngle The starting angle.
          */
         inline void setStartingAngle( uint8 angle );
         
         /**
          * Get starting angle, should only be used if map 
          * contains route_origin feature.
          * @see setStartingAngle for angle definition.
          * @return The starting angle.
          */
         inline uint8 getStartingAngle() const;
         
         /**
          * Set drivingOnRightSide.
          * @param drivingOnRightSide The side to drive on.
          */
         inline void setDrivingOnRightSide( bool drivingOnRightSide );
         
         /**
          * Get drivingOnRightSide in traffic.
          * @return drivingOnRightSide in traffic.
          */
         inline bool getDrivingOnRightSide() const;

         /**
          * Set initialUTurn, should only be used if map 
          * contains route_origin feature.
          * @param initialUTurn If make U-Turn at route_origin.
          */
         inline void setInitialUTurn( bool initialUTurn );
         
         /**
          * Get initialUTurn, should only be used if map 
          * contains route_origin feature.
          * @return initialUTurn.
          */
         inline bool getInitialUTurn() const;

         /**
          * Sets the route settings in this GfxFeature map to those of
          * another map.
          */
         void mergeRouteSettings( const GfxFeatureMap* otherMap );
      //@}
     
      /**
       *   Remove duplicate features. XXX: Very costly.
       */
      void removeDuplicates();
      
      /**
       *    Get unique features of a certain type.
       *    Keeps the feature order. XXX: Very costly.
       */
      void getUniqueFeatures( vector<const GfxFeature*>& uniques,
                              GfxFeature::gfxFeatureType type ) const; 
         
      /**
        *   Get the first Feature in this map and remove it!
        *   @return The first GfxFeature in this map, will be removed
        *           from this map. NULL will be returned if no features
        *           left.
        */
      inline GfxFeature* getAndRemoveLastFeature();

      /**
       *    Get the gfxmap as "simple map data", i.e. a very simple
       *    map format suitable for primitive clients.
       *    @param version The version of simple map data to be used.
       *                   Currently only version 1 exists.
       *    @param outSize [OUT] Will be set to the number of bytes
       *                   of the resulting buffer.
       *    @return  A new byte array containing the map.
       *             Must be deleted by the caller. Contains outSize
       *             number of bytes. If invalid version is supplied
       *             then the buffer returned is NULL.
       */
      byte* getAsSimpleMapData( int version,
                                uint32& outSize ) const;
      
      /**
       *  Reverse the order of the features.
       */
      void reverseFeatures();

 
      /// const iterator
      typedef GfxFeatureVector::const_iterator const_iterator;


      /// const begin
      const_iterator begin() const {
         return m_features.begin();
      }

      /// const end
      const_iterator end() const {
         return m_features.end();
      }

      /// iterator
      typedef GfxFeatureVector::iterator iterator;

      /// begin
      iterator begin() {
         return m_features.begin();
      }

      /// end
      iterator end() { 
         return m_features.end();
      }

   
   private:
      /**
        *   Get the first Feature in this map and remove it!
        *   @return The first GfxFeature in this map, will be removed
        *           from this map. NULL will be returned if no features
        *           left.
        */
      GfxFeature* getAndRemoveFirstFeature();
   
      /**
       *    The boundingbox for the map.
       */
      MC2BoundingBox m_bbox;
      
      /**
       *    The height and width of the screen.
       */
      ScreenSize m_screenSize;

      /**
       *    The scale of this map. Could be calculated from m_bbox and
       *    m_screenBBox, but stored to be more flexible. The scale-value
       *    represents the number of meters / pixel.
       */
      uint32 m_scaleLevel;
      
      /**
       *    Vector of features.
       */
      GfxFeatureVector m_features;

      /**
       *    The current size of the features.
       */
      uint32 m_featureSize;

      /**
       * The transportation type of this GfxFeatureMap.
       * Note that this member only makes sense when this map contains
       * a route_origin feature.
       */
      ItemTypes::transportation_t m_transportationType;

      /**
       * The starting angle of the route.
       * Note that this member only makes sense when this map contains
       * a route_origin feature.
       */
      uint8 m_startingAngle;

      /**
       * If drivingOnRightSide in traffic.
       * Note that this member only makes sense when this map contains
       * a route_origin feature.
       */
      bool m_drivingOnRightSide;

      /**
       * If initialUTurn at route_origin.
       * Note that this member only makes sense when this map contains
       * a route_origin feature.
       */
      bool m_initialUTurn;
};

// =======================================================================
//                                     Implementation of inlined methods =

inline uint32 
GfxFeatureMap::getScaleLevel() const
{
   return (m_scaleLevel);
}

inline void 
GfxFeatureMap::setScaleLevel(uint32 newScale)
{
   m_scaleLevel = newScale;
}

inline void 
GfxFeatureMap::setTransportationType(
            ItemTypes::transportation_t transpType )
{
   m_transportationType = transpType;
}

inline ItemTypes::transportation_t 
GfxFeatureMap::getTransportationType() const {
   return m_transportationType;
}

inline void 
GfxFeatureMap::setStartingAngle( uint8 angle ) {
   m_startingAngle = angle;
}

inline uint8 
GfxFeatureMap::getStartingAngle() const {
   return m_startingAngle;
}

inline void 
GfxFeatureMap::setDrivingOnRightSide( bool drivingOnRightSide ) {
   m_drivingOnRightSide = drivingOnRightSide;
}

inline bool 
GfxFeatureMap::getDrivingOnRightSide() const {
   return m_drivingOnRightSide;
}

inline void 
GfxFeatureMap::setInitialUTurn( bool initialUTurn ) {
   m_initialUTurn = initialUTurn;
}

inline bool 
GfxFeatureMap::getInitialUTurn() const {
   return m_initialUTurn;
}

inline GfxFeature* 
GfxFeatureMap::getAndRemoveLastFeature() {
   GfxFeature* returnValue = NULL;
   if (m_features.size() > 0) {
      returnValue =  m_features.back();
      m_features.erase( m_features.end() - 1 );
      m_featureSize -= returnValue->getSize();
   }
   return (returnValue);
}

inline ScreenSize::ValueType
GfxFeatureMap::getScreenX() const {
   return m_screenSize.getWidth();
}

inline ScreenSize::ValueType
GfxFeatureMap::getScreenY() const {
   return m_screenSize.getHeight();
}

inline const ScreenSize&
GfxFeatureMap::getScreenSize() const {
   return m_screenSize;
}


#endif // GFX_FEATURE_MAP_H

