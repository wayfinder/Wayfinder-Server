/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_MAP_PARAMS_H
#define TILE_MAP_PARAMS_H

#include "TileMapConfig.h"

#include "LangTypes.h"
#include "TileMapTypes.h"
#include "MC2SimpleString.h"

#define TILEMAP_CODE_CHARS \
             "!()*+-<>@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^abcdefghijklmnopqrstuvwxyz"

#include "RouteID.h"

/**
 *   Class which should be used when requesting tile maps
 *   from the Cache or similar.
 */
class TileMapParams {
   public:
      /**
       *  Empty constructor.
       */
      inline TileMapParams();
      
      /**
       *  Creates a TileMapParams from the supplied string.
       *
       *  @param knownUnknown If paramString is known to be unknown, just
       *                      set valid nothing else if not valid map tile.
       *                      Default false.
       */
      TileMapParams( const char* paramString, bool knownUnknown = false );

      /**
       *  Creates a TileMapParams from the supplied string.
       *  
       *  @param knownUnknown If paramString is known to be unknown, just
       *                      set valid nothing else if not valid map tile.
       *                      Default false.
       */
      TileMapParams( const MC2SimpleString& paramString, 
                     bool knownUnknown = false );

      /**
       *   Copy constructor.
       */
      TileMapParams( const TileMapParams& other );
       
      /**
       *  Creates a TileMapDesc from the supplied parameters.
       *  @param serverPrefix Prefix given by server when requesting
       *                      parameter block. Only six bits are sent.
       *  @param gzip         True if gzip may be used.
       *  @param layer        The layer number.
       *  @param mapOrStrings 
       */
      inline TileMapParams( uint32 serverPrefix,
                            int gzip, 
                            int layer,
                            TileMapTypes::tileMap_t mapOrStrings,
                            int importanceNbr,
                            LangTypes::language_t langType,
                            int32 tileIndexLat,
                            int32 tileIndexLon,
                            int detailLevel,
                            const RouteID* routeID = NULL);

      /**
       *  Destructor.
       */
      ~TileMapParams() {
         delete m_paramString;
         delete m_routeID;
      }

      /**
       *  Sets the params.
       *  @param serverPrefix Prefix given by server when requesting
       *                      parameter block. Only six bits are sent.
       *  @param gzip         True if gzip may be used.
       *  @param layer        The layer number.
       *  @param mapOrStrings 
       */
      inline void setParams( uint32 serverPrefix,
                             int gzip, 
                             int layer,
                             TileMapTypes::tileMap_t mapOrStrings,
                             int importanceNbr,
                             LangTypes::language_t langType,
                             int32 tileIndexLat,
                             int32 tileIndexLon,
                             int detailLevel,
                             const RouteID* routeID = NULL);
      
      /**
       *  Copy the attributes from the TileMapParams.
       */
      inline void copyAttributes( const TileMapParams& other );

      /**
       *   Assignment operator.
       */
      inline TileMapParams& operator=(const TileMapParams& other);

      /**
       *   Compares all the members of the params except
       *   for the string to be able to put the params
       *   into e.g. a set.
       */
      inline bool operator<( const TileMapParams& other ) const;
      
      /**
       *   Returns true if the parameters were parsed ok.
       */
      bool getValid() const;

      /**
       *   Returns the string representation of the TileMapDesc.
       */
      const MC2SimpleString& getAsString() const { 
         if ( m_paramString == NULL ) {
            const_cast<TileMapParams*> (this)->updateParamString();
         }
         return *m_paramString; 
      }

      /**
       *   Returns the string representation of the TileMapDesc.
       */
      operator const MC2SimpleString&() const { return getAsString(); }
     
      // -- Specifics
      
      
      /**
       *   @return Whether to use gzip or not.
       */
      inline bool useGZip() const;

      /**
       *   Set whether to use gzip or not.
       */
      inline void setUseGZip(bool val);
      
      /**
       *   @return Layer.
       */
      int getLayer() const;
      
      /**
       *   Set the layer id.
       */
      inline void setLayer( int layerID );
      
      /**
       *   @return TileMap type (map or strings).
       */
      TileMapTypes::tileMap_t getTileMapType() const;

      /**
       *   Sets the type of the params.
       *   WARNING! Updates the paramstring too, may be slow.
       */
      void setTileMapType(TileMapTypes::tileMap_t newType);
      
      /**
       *   @return Importance number.
       */
      inline int getImportanceNbr() const;

      /**
       *   Set the importance nbr.
       */   
      inline void setImportanceNbr( int importanceNbr );

      /**
       *   @return Language type.
       */
      inline LangTypes::language_t getLanguageType() const;

      /**
       *   Sets new language.
       *   WARNING! Updates the paramstring too, may be slow.
       */
      inline void setLanguageType(LangTypes::language_t lang);
      
      /**
       *   @return Tile index latitude.
       */
      inline int32 getTileIndexLat() const;
      
      /**
       *   @return Tile index longitude.
       */
      inline int32 getTileIndexLon() const;

      /**
       *   @return Detail level.
       */
      inline int getDetailLevel() const;

      /**
       *   @return Server prefix.
       */
      inline uint32 getServerPrefix() const;

      /**
       *   @return The route id.
       */
      inline const RouteID* getRouteID() const;

      /**
       *    Dump the param.
       */
      friend ostream& operator << ( ostream& stream, 
                                    const TileMapParams& params );

   private:
      
      /**
       *    Updates the parameter string.
       */
      void updateParamString();

      /**
       *    Parse the param string and set the resulting member
       *    variables.
       */      
      void parseParamString( const char* paramString, bool knownUnknown );
      
      /**
       *    Parameter string.
       */
      MC2SimpleString* m_paramString;
      
      /**
       *    Server prefix (version). Only five bits are sent.
       *    (used to be 6, but highest was hijacked for quirkymode.)
       */
      uint32 m_serverPrefix;
      
      /**
       *    Use gzip?
       */
      int m_gzip;
      
      /**
       *    Layer.
       */
      int m_layer;
      
      /**
       *    Type of map - strings or vectors.
       */
      TileMapTypes::tileMap_t m_mapOrStrings;
      
      /**
       *    Importance nbr.
       */
      int m_importanceNbr;

      /**
       *    Language type.
       */
      LangTypes::language_t m_langType;
      
      /**
       *    Tile index latitude.
       */
      int32 m_tileIndexLat;
      
      /**
       *    Tile index longitude.
       */
      int32 m_tileIndexLon;

      /**
       *    The zoom or detail level of the map.
       */
      int m_detailLevel;

      /// Sorted code characters. 
      static const char * const c_sortedCodeChars;

      static const uint32 c_sortedCodeCharsLen;
      
      /**
       *    True if the parameters have been parsed ok.
       */
      int m_valid;
      
      /// Route ID if this is a route param
      RouteID* m_routeID;
};

// Implementation of inlined methods. ---

#define CHECK_RET_LESS(x) if ( this->x != other.x ) {\
                             return this->x < other.x; }

inline bool
TileMapParams::operator<( const TileMapParams& other ) const
{
   CHECK_RET_LESS( m_tileIndexLat );
   CHECK_RET_LESS( m_tileIndexLon );
   CHECK_RET_LESS( m_serverPrefix );
   CHECK_RET_LESS( m_gzip );
   CHECK_RET_LESS( m_layer );
   CHECK_RET_LESS( m_mapOrStrings );
   CHECK_RET_LESS( m_importanceNbr );
   CHECK_RET_LESS( m_langType );
   CHECK_RET_LESS( m_detailLevel );
   
   if ( ( m_routeID != NULL ) && ( other.m_routeID != NULL ) ) {
      // Compare the values of routeIDs if none is null
      CHECK_RET_LESS( m_routeID->toNav2Int64() );
   } else {
      // Compare pointers when one is or both are NULL
      CHECK_RET_LESS( m_routeID );
   }
   // Same
   return false;
}

#undef CHECK_RET_LESS

inline 
TileMapParams::TileMapParams() : 
   m_paramString(NULL),
   m_routeID(NULL)
{

}

inline void
TileMapParams::setParams( uint32 serverPrefix,
                         int gzip, 
                         int layer,
                         TileMapTypes::tileMap_t mapOrStrings,
                         int importanceNbr,
                         LangTypes::language_t langType,
                         int32 tileIndexLat,
                         int32 tileIndexLon,
                         int detailLevel,
                         const RouteID* routeID) 
{
   delete m_paramString;
   m_paramString = NULL;
   m_serverPrefix = serverPrefix;
   m_gzip = gzip;
   m_layer = layer;
   m_mapOrStrings = mapOrStrings;
   m_importanceNbr = importanceNbr;
   m_langType = langType;
   m_tileIndexLat = tileIndexLat;
   m_tileIndexLon = tileIndexLon;
   m_detailLevel = detailLevel;
   m_valid = true;
   
   if ( routeID != NULL ) {
      delete m_routeID;
      m_routeID = new RouteID( *routeID );
   }
}

inline void 
TileMapParams::copyAttributes( const TileMapParams& other )
{
   setParams( other.m_serverPrefix,
              other.m_gzip,
              other.m_layer,
              other.m_mapOrStrings,
              other.m_importanceNbr,
              other.m_langType,
              other.m_tileIndexLat,
              other.m_tileIndexLon,
              other.m_detailLevel,
              other.m_routeID );
}

inline
TileMapParams::TileMapParams( uint32 serverPrefix,
                              int gzip, 
                              int layer,
                              TileMapTypes::tileMap_t mapOrStrings,
                              int importanceNbr,
                              LangTypes::language_t langType,
                              int32 tileIndexLat,
                              int32 tileIndexLon,
                              int detailLevel,
                              const RouteID* routeID) :
   m_paramString(NULL),
   m_routeID(NULL)
{
   setParams( serverPrefix, gzip, layer, mapOrStrings, importanceNbr, 
              langType, tileIndexLat, tileIndexLon, detailLevel, routeID );
}

inline TileMapParams&
TileMapParams::operator = ( const TileMapParams& other ) 
{
   copyAttributes( other );
   return *this;
}

inline bool 
TileMapParams::useGZip() const
{
   return m_gzip;
}

inline void
TileMapParams::setUseGZip(bool val)
{
   m_gzip = val;
   delete m_paramString;
   m_paramString = NULL;
}

inline int
TileMapParams::getLayer() const
{
   return m_layer;
}

inline void 
TileMapParams::setLayer( int layerID )
{
   m_layer = layerID;
   delete m_paramString;
   m_paramString = NULL;
}

inline TileMapTypes::tileMap_t 
TileMapParams::getTileMapType() const
{
   return m_mapOrStrings;
}

inline void
TileMapParams::setTileMapType(TileMapTypes::tileMap_t type)
{
   m_mapOrStrings = type;
   delete m_paramString;
   m_paramString = NULL;
}

inline int 
TileMapParams::getImportanceNbr() const
{
   return m_importanceNbr;
}

inline void
TileMapParams::setImportanceNbr( int importanceNbr )
{
   m_importanceNbr = importanceNbr;
   delete m_paramString;
   m_paramString = NULL;
}

inline LangTypes::language_t 
TileMapParams::getLanguageType() const
{
   return m_langType;
}

inline void
TileMapParams::setLanguageType(LangTypes::language_t type)
{
   m_langType = type;
   delete m_paramString;
   m_paramString = NULL;
}

inline int32 
TileMapParams::getTileIndexLat() const
{
   return m_tileIndexLat;
}

inline int32 
TileMapParams::getTileIndexLon() const
{
   return m_tileIndexLon;
}
      
inline int 
TileMapParams::getDetailLevel() const
{
   return m_detailLevel;
}
      
inline uint32 
TileMapParams::getServerPrefix() const
{
   return m_serverPrefix;
}
      
inline const RouteID* 
TileMapParams::getRouteID() const
{
   return m_routeID;
}

inline bool
TileMapParams::getValid() const
{
   return m_valid;
}

#endif // TILE_MAP_PARAMS_H
