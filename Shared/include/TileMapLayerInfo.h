/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILEMAPLAYERINFO_H
#define TILEMAPLAYERINFO_H

#include "config.h"

#include "MC2SimpleString.h"

#include <vector>
#include <set>

class BitBuffer;

/**
 *   Class containing information about a layer.
 */
class TileMapLayerInfo {
public:
   /// Creates an empty TileMapLayerInfo
   TileMapLayerInfo();

#ifdef MC2_SYSTEM
   /// Creates a full TileMapLayerInfo ( mc2 use )
   inline TileMapLayerInfo( uint32 id,
                            const MC2SimpleString& name,
                            uint32 updatePeriodMin,
                            bool optional,
                            bool visibleByDefault,
                            bool alwaysFetchStrings,
                            bool affectedByACPMode = false,
                            bool fetchLayerWhenACPEnabled = false,
                            bool fetchLayerWhenACPDisabled = false
                            ) :
      m_id(id),
      m_name(name),
      m_updatePeriod( updatePeriodMin ),
      m_transient( updatePeriodMin ),
      m_isOptional( optional ),
      m_serverOverride( 0 ),
      // Assume that it is present in TMFD since it is created there.
      m_presentInTileMapFormatDesc( true ),
      m_visible( visibleByDefault ),
      m_alwaysFetchStrings( alwaysFetchStrings ),
      m_affectedByACPMode( affectedByACPMode ),
      m_fetchLayerWhenACPEnabled( fetchLayerWhenACPEnabled ),
      m_fetchLayerWhenACPDisabled( fetchLayerWhenACPDisabled ) {}
#endif

   // Allowed to be set by the user
   
   /// Fills in the period. 0 for non-updateable layers.
   bool setUpdatePeriodMinutes( uint32 period );
   /// Gets the update period
   inline uint32 getUpdatePeriodMinutes() const;   
   /// Sets if the layer should be shown or not
   bool setVisible( bool visible );
   /// Returns true if the layer should be shown
   inline bool isVisible() const;
   
   // End allowed to be set by the user

   /// Gets if the layer is optional and present, i.e. should be displayed.
   inline bool isOptionalAndPresent() const;

   /// Gets the name (UTF-8)
   inline const MC2SimpleString& getName() const;
   
   
   /// Sets if the layer is optional. @return modified
   bool setOptional( int optional );
   /// Return if the layer is optional
   inline bool isOptional() const;
   /// Sets if the layer is present in TMFD. @return modified
   bool setPresent( bool present );
   /// Returns if the layer is present in the TMFD
   inline bool isPresent() const;
   /// Gets the id 
   inline uint32 getID() const;
   /// Returns true if the layer is transient and should not be saved in cache
   inline bool isTransient() const;

   /// Sets the server override number and returns true if changed.
   bool setServerOverrideNumber( uint8 nbr );
   /// Returns the server override number.
   inline uint8 getServerOverrideNumber() const;
   /// True if the strings should be fetched auto or when clicked.
   inline bool alwaysFetchStrings() const;

   /**
    *    Set if the ACP mode setting (enable/disable Access Controlled POIs)
    *    should apply for this layer.
    */
   void setAffectedByACPMode( bool affected );
   
   /**
    *    Set if the ACP mode setting (enable/disable Access Controlled POIs)
    *    should apply for this layer.
    */
   bool isAffectedByACPMode() const;

   /**
    *    Only valid if isAffectedByACPMode() returns true!
    *   
    *    Set if the layer should be fetched when ACP mode is enabled.
    *    This setting should override any other visibility settings.
    */
   void setFetchLayerWhenACPEnabled( bool fetch );

   /**
    *    Only valid if isAffectedByACPMode() returns true!
    *   
    *    Set if the layer should be fetched when ACP mode is enabled.
    *    This setting should override any other visibility settings.
    */
   bool getFetchLayerWhenACPEnabled() const;

   /**
    *    Only valid if isAffectedByACPMode() returns true!
    *   
    *    Set if the layer should be fetched when ACP mode is disabled.
    *    This setting should override any other visibility settings.
    */
   void setFetchLayerWhenACPDisabled( bool fetch );

   /**
    *    Only valid if isAffectedByACPMode() returns true!
    *   
    *    Get if the layer should be fetched when ACP mode is disabled.
    *    This setting should override any other visibility settings.
    */
   bool getFetchLayerWhenACPDisabled() const;


   /// Fills in a new TileMapLayerInfo
   void setIDAndName( uint32 id,
                      const MC2SimpleString& name );
   
   /// Saves all contents in the buffer.
   int save( BitBuffer& buf ) const;
   /// Loads all contents from the buffer.
   int load( BitBuffer& buf );
   /// Return the size of the data in a buffer.
   int getSizeInDataBuffer() const;
   
private:
   /// The id of the layer
   uint32 m_id;
   /// The name of the layer
   MC2SimpleString m_name;
   /// Suggested update period for updatable layers.
   uint32 m_updatePeriod;
   /// True if the layer is transient. If 1 then don't save in file cache.
   bool m_transient;
   /// True if the layer is optional
   bool m_isOptional;
   /// Server override number
   uint8 m_serverOverride;
   /// True if the layer is present in the current TileMapFormatDesc
   bool m_presentInTileMapFormatDesc;
   /// True if the layer is visible
   bool m_visible;
   /// True if the strings should be fetched without click
   bool m_alwaysFetchStrings;

   /**
    *    If the ACP mode setting (enable/disable Access Controlled POIs)
    *    should apply for this layer.
    */
   bool m_affectedByACPMode;

   /**
    *    Only valid if m_affectedByACPMode is true!
    *   
    *    If the layer should be fetched when ACP mode is enabled.
    *    This setting should override any other visibility settings.
    */
   bool m_fetchLayerWhenACPEnabled;
   
   /**
    *    Only valid if m_affectedByACPMode is true!
    *   
    *    If the layer should be fetched when ACP mode is disabled.
    *    This setting should override any other visibility settings.
    */
   bool m_fetchLayerWhenACPDisabled;

};

inline
bool
TileMapLayerInfo::isVisible() const
{
   return m_visible;
}

inline
bool
TileMapLayerInfo::isTransient() const
{
   return m_transient;
}

inline bool
TileMapLayerInfo::isOptional() const
{
   return m_isOptional;
}

inline uint8
TileMapLayerInfo::getServerOverrideNumber() const
{
   return m_serverOverride;
}

inline const MC2SimpleString& 
TileMapLayerInfo::getName() const
{
   return m_name;
}

inline uint32
TileMapLayerInfo::getID() const
{
   return m_id;
}

inline uint32 
TileMapLayerInfo::getUpdatePeriodMinutes() const
{
   return m_updatePeriod;
}

inline bool
TileMapLayerInfo::isOptionalAndPresent() const
{
   return m_isOptional && m_presentInTileMapFormatDesc;
}

inline bool
TileMapLayerInfo::isPresent() const
{
   return m_presentInTileMapFormatDesc;
}

inline bool
TileMapLayerInfo::alwaysFetchStrings() const
{
   return m_alwaysFetchStrings;
}


/**
 *    Container for the TileMapLayerInfos
 */
class TileMapLayerInfoVector : public std::vector<TileMapLayerInfo> {
public:
   
   /**
    *   Updates the layers from the client settings, this is
    *   assumed to be the vector from the TileMapFormatDesc.
    */
   bool updateLayers( TileMapLayerInfoVector& clientVector,
                      const std::set<int>& existingLayers );

   /**
    *   Puts the id:s of the layers to be displayed into
    *   the set.
    *   @return true if the layers to display have changed.
    */
   bool updateLayersToDisplay( std::set<int>& layerIDs ) const;

   /// Returns the info for the layer with the supplied id
   const TileMapLayerInfo* getLayerInfo( uint32 layerID ) const;
   
   /// Saves all contents in the buffer.
   int save( BitBuffer& buf ) const;
   /// Loads all contents from the buffer.
   int load( BitBuffer& buf );
   /// Return the size of the data in a buffer.
   int getSizeInDataBuffer() const;   
};


#endif

