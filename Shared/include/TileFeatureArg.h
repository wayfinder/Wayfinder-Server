/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_FEATURE_ARG_H
#define TILE_FEATURE_ARG_H

#include "TileMapConfig.h"
#include "BitBuffer.h"
#include "MC2SimpleString.h"
#include "MC2BoundingBox.h"
#include "TileArgNames.h"

#include "TileMapCoord.h"

#include <vector>
#include <map>

class TileMap;

#define VALID_TILE_COLOR( a ) (((a & 0x1ffffff) != 0x1ffffff) ? true : false)

class CoordArg;
class CoordsArg;
class SimpleArg;
class StringArg;
struct ltArgPtr;

class TileFeatureArg {
public:

   enum tileFeatureArg_t {
      /// Arg is SimpleArg
      simpleArg = 0,
      /// Arg is CoordArg
      coordArg  = 1,
      /// Arg is CoordsArg
      coordsArg = 2,
      /// Arg is StringArg
      stringArg = 3
   };
   
   TileFeatureArg( TileArgNames::tileArgName_t name ) : m_name( name ) {
   }

   /// Instead of dynamic cast in Symbian
   inline static const SimpleArg* simpleArgCast(
                                 const TileFeatureArg* probSimple);
   
   /// Instead of dynamic cast in Symbian
   inline static const StringArg* stringArgCast(
                                 const TileFeatureArg* probString);

   /// Instead of dynamic cast in Symbian
   inline static const CoordsArg* coordsArgCast(
                                 const TileFeatureArg* probCoords);
   
   
   virtual ~TileFeatureArg() {}

#ifdef MC2_SYSTEM      
      /**
       *    Abstract save method.
       *    @param   buf      The buffer to save to.
       *    @param   tileMap  The tile map.
       *    @param   prevArg  The adjacent argument of the previous 
       *                      TileFeature in the map.
       *                      NULL if the previous TileFeature in 
       *                      the map was not of the same type as this one.
       */
      virtual bool save( BitBuffer& buf, 
                         const TileMap& tileMap,
                         const TileFeatureArg* prevArg ) const = 0;
#endif
      
      /**
       *    Abstract load method.
       *    @param   buf      The buffer to save to.
       *    @param   tileMap  The tile map.
       *    @param   prevArg  The adjacent argument of the previous 
       *                      TileFeature in the map.
       *                      NULL if the previous TileFeature in 
       *                      the map was not of the same type as this one.
       */
      virtual bool load( BitBuffer& buf, 
                         TileMap& tileMap, 
                         const TileFeatureArg* prevArg ) = 0;
      
      virtual TileFeatureArg* clone() = 0;
      
      inline TileArgNames::tileArgName_t getName() const;

      virtual tileFeatureArg_t getArgType() const = 0;

#ifdef MC2_SYSTEM   
      virtual void dump( ostream& stream ) const;

      /**
       *    Save the full data of the argument.
       *    TileMapFormatDesc will not be needed to load the
       *    data afterwards.
       *    @param   buf   The buffer to save to.
       */
      void saveFullArg( BitBuffer& buf ) const;
#endif   

      /**
       *    Load the full data of the argument.
       *    TileMapFormatDesc is not needed for loading.
       *    @param   buf   The buffer to load from.
       *    @return  The a new TileFeatureArg of the correct dynamic 
       *             type.
       */
      static TileFeatureArg* loadFullArg( BitBuffer& buf );
 
   protected:
      /// The name or type really.
      TileArgNames::tileArgName_t m_name;
};

class SimpleArg : public TileFeatureArg { 
   public:
      friend struct ltArgPtr;

      /**
       *   @param argName Name of the argument.
       *   @param nbrBits Number of bits to send for the arg. (1-32)
       */
      SimpleArg( TileArgNames::tileArgName_t argName, byte nbrBits ) :
         TileFeatureArg( argName ), m_size( nbrBits ) {
#ifdef MC2_SYSTEM
            setValue( 0 ); // For crc calculation in server.
#endif            
         }

      /**
       *   Sets the value.
       */
      void setValue( uint32 value );
      
      void setValue( const vector<uint32>& valueByScaleIdx );

      uint32 getValue( int scaleIdx = 0 ) const;

#ifdef MC2_SYSTEM      
      /**
       *   Saves the arg in a databuffer.
       *   Uses 1,2,3 or 4 bytes.
       *   
       *   @param   buf      The buffer to save to.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool save( BitBuffer& buf, const TileMap& tileMap,
                 const TileFeatureArg* prevArg ) const;

      virtual void dump( ostream& stream ) const;
#endif
      
      /**
       *   Loads the arg.
       *   Uses 1,2,3 or 4 bytes.
       *   
       *   @param   buf      The buffer to load from.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool load( BitBuffer& buf, TileMap& tileMap,
                 const TileFeatureArg* prevArg );

      virtual TileFeatureArg* clone();

      inline byte getSize() const;
      
   tileFeatureArg_t getArgType() const {
      return simpleArg;
   }
   
   private:

      vector<uint32> m_valueByScaleIdx;
      byte m_size;
};

/**
 *    Argument containing a string.
 */
class StringArg : public TileFeatureArg 
{ 
   public:
      friend struct ltArgPtr;

      /**
       *   @param argName Name of the argument.
       */
      StringArg( TileArgNames::tileArgName_t argName ) :
         TileFeatureArg( argName ) {}

      /**
       *   Sets the string.
       */
      void setValue( const vector<MC2SimpleString>& strByScaleIdx );
      void setValue( const MC2SimpleString& str );

      const MC2SimpleString& getValue( int scaleIdx = 0 ) const;

#ifdef MC2_SYSTEM
      virtual void dump( ostream& stream ) const;
      
      /**
       *   Saves the arg in a databuffer.
       *   
       *   @param   buf      The buffer to save to.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool save( BitBuffer& buf, const TileMap& tileMap,
                 const TileFeatureArg* prevArg ) const;
#endif
      
      /**
       *   Loads the arg.
       *   
       *   @param   buf      The buffer to load from.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool load( BitBuffer& buf, TileMap& tileMap,
                 const TileFeatureArg* prevArg );

      virtual TileFeatureArg* clone();



      tileFeatureArg_t getArgType() const {
         return stringArg;
      }
   
   private:
     
      vector<MC2SimpleString> m_stringByScaleIdx;
};

class CoordArg : public TileFeatureArg
{
   public:
      CoordArg( TileArgNames::tileArgName_t argName ) : 
                                       TileFeatureArg( argName ) {
#ifdef MC2_SYSTEM
      m_coord.setCoord(0, 0); // For crc calculation in server.
#endif                                          
      }

#ifdef MC2_SYSTEM
      /**
       *    Set the coordinate.
       *      
       *    @param   lat      Latitude.
       *    @param   lon      Longitude.
       *    @param   tileMap  The TileMap containing the argument.
       */
      void setCoord( int32 lat, int32 lon, const TileMap& tileMap );

      /**
       *   Save method.
       *   
       *   @param   buf      The buffer to save to.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool save( BitBuffer& buf, const TileMap& tileMap,
                 const TileFeatureArg* prevArg ) const;
   
      virtual void dump( ostream& stream ) const;
#endif

      /**
       *   Load method.
       *   
       *   @param   buf      The buffer to load from.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool load( BitBuffer& buf, TileMap& tileMap,
                 const TileFeatureArg* prevArg );

      virtual TileFeatureArg* clone();
   
   tileFeatureArg_t getArgType() const {
      return coordArg;
   }

   const TileMapCoord& getCoord() const {
      return m_coord;
   }
   
   private: 
   
      TileMapCoord m_coord;
};

class CoordsArg : public TileFeatureArg
{
public:
   /**
    *   Creates a new coords arg with a pointer to the
    *   collection of all coordinates.
    */
   CoordsArg( TileMapCoords* allCoords,
              TileArgNames::tileArgName_t argName ) :
      TileFeatureArg( argName ), m_allCoords(allCoords) {
      // Means that it should be updated at addCoord.
      m_startCoordIdx = MAX_UINT32;
      m_endCoordIdx   = MAX_UINT32;
   }

#ifdef MC2_SYSTEM      
      /**
       *    Add the coordinate.
       *      
       *    
       *    @param   lat      Latitude.
       *    @param   lon      Longitude.
       *    @param   tileMap  The TileMap containing the argument.
       */
      void addCoord( int32 lat, 
                     int32 lon, 
                     TileMap& tileMap );

      /**
       *   Save method.
       *   
       *   @param   buf      The buffer to save to.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool save( BitBuffer& buf, const TileMap& tileMap,
                 const TileFeatureArg* prevArg ) const;
   
      virtual void dump( ostream& stream ) const;
#endif      

      /**
       *   Load method.
       *   
       *   @param   buf      The buffer to load from.
       *   @param   tileMap  The tile map.
       *   @param   prevArg  The adjacent argument of the previous 
       *                     TileFeature in the map.
       *                     NULL if the previous TileFeature in 
       *                     the map was not of the same type as this one.
       */
      bool load( BitBuffer& buf, TileMap& tileMap,
                 const TileFeatureArg* prevArg );

      virtual TileFeatureArg* clone();

      typedef TileMapCoords::iterator iterator;
      typedef TileMapCoords::const_iterator const_iterator;
      typedef TileMapCoords::value_type value_type;

      inline const_iterator begin() const;

      inline const_iterator end() const;

      inline iterator begin();

      inline iterator end();
      
      inline const TileMapCoord& front() const;

      inline const TileMapCoord& back() const;

      inline int size() const;

      inline const MC2BoundingBox& getBBox() const;



   tileFeatureArg_t getArgType() const {
      return coordsArg;
   }
private:
   /// Adds a coordinate
   inline void push_back( const TileMapCoord& coord );
   /// The bounding box of the coordinates.
   MC2BoundingBox m_bbox;
   /// The pointer to the common vector of coordinates.
   TileMapCoords* m_allCoords;
   /// Start index of the coords
   uint32 m_startCoordIdx;
   /// End index of the coords.
   uint32 m_endCoordIdx;
};

/**
 *    Functor performing "less than" for TileFeatureArg pointers.
 */
struct ltArgPtr
{
   /**
    *    Less than.
    */
   bool operator()( const TileFeatureArg* arg1, 
                    const TileFeatureArg* arg2 ) const;
};

/**
 *    TileArgContainer that can be used for sharing common TileArgs.
 */
class TileArgContainer {

   public:

      /**
       *    Destructor. Deletes all the TileArgs.
       */
      ~TileArgContainer();

#ifdef MC2_SYSTEM      
      /**
       *    Save to buffer.
       *    @param   buf   The buffer.
       */
      void save( BitBuffer& buf ) const;
#endif

      /**
       *    Load from buffer.
       *    @param   buf   The buffer.
       */
      void load( BitBuffer& buf );

      /**
       *    Add the argument to the container and get an index to the 
       *    argument. Not supplied argument now is the belongings of 
       *    the TileArgContainer.
       *
       *    @param   arg   The TileArg to add to the TileArgContainer.
       *                   NB! The arg is after this owned by the
       *                   TileArgContainer which may delete immediately
       *                   after.
       *    @return  The index to the TileFeatureArg. This should 
       *             afterwards be used instead of the argument passed 
       *             as parameter.
       */
      int addArg( TileFeatureArg* arg );

      /**
       *    Get the argument at the specified index.
       *    @param   index The index.
       *    @return  The argument.
       */
      TileFeatureArg* getArg( int index );
      
      /**
       *    Get the index for the specified TileArg.
       *    @param  arg The TileArg.
       *    @return  The index of the TileArg if found in the container,
       *             otherwise -1.
       */
      int getArgIndex( TileFeatureArg* arg ) const;
      
   private: 

      /**
       *    Map containing TileFeatureArg* as key and index as value.
       */
      typedef map< TileFeatureArg*, int, ltArgPtr > indexByArg_t;
      
      /**
       *    Map containing TileFeatureArg* as key and index as value.
       */
      indexByArg_t m_indexByArg;      

      /**
       *    Vector containing TileFeatureArg* order by index.
       */
      vector< TileFeatureArg* > m_argByIndex;
};

// --- Implementation of inlined methods ---

inline const SimpleArg*
TileFeatureArg::simpleArgCast(const TileFeatureArg* probSimple)
{
   if ( ( probSimple != NULL ) &&
        ( probSimple->getArgType() == simpleArg ) ) {
      return static_cast<const SimpleArg*>(probSimple);
   } else {
      return NULL;
   }
}

inline const StringArg*
TileFeatureArg::stringArgCast(const TileFeatureArg* probString)
{
   if ( ( probString != NULL ) &&
        ( probString->getArgType() == stringArg ) ) {
      return static_cast<const StringArg*>(probString);
   } else {
      return NULL;
   }
}

inline const CoordsArg*
TileFeatureArg::coordsArgCast(const TileFeatureArg* probCoords)
{
   if ( ( probCoords != NULL ) &&
        ( probCoords->getArgType() == coordsArg ) ) {
      return static_cast<const CoordsArg*>(probCoords);
   } else {
      return NULL;
   }
}

inline 
TileArgNames::tileArgName_t 
TileFeatureArg::getName() const
{
   return m_name;
}

inline byte 
SimpleArg::getSize() const
{
   return m_size;
}


inline TileMapCoords::iterator 
CoordsArg::begin() 
{
   return m_allCoords->begin() + m_startCoordIdx;
}

inline TileMapCoords::iterator 
CoordsArg::end() 
{
   return m_allCoords->begin() + m_endCoordIdx;
}


inline TileMapCoords::const_iterator 
CoordsArg::begin() const 
{
   return m_allCoords->begin() + m_startCoordIdx;
}

inline TileMapCoords::const_iterator 
CoordsArg::end() const 
{
   return m_allCoords->begin() + m_endCoordIdx;
}

inline const TileMapCoord& 
CoordsArg::front() const
{
   return *begin();
}

inline const TileMapCoord& 
CoordsArg::back() const
{
   const_iterator endIt = end();
   return *(--endIt);
}

inline int 
CoordsArg::size() const 
{
   return m_endCoordIdx - m_startCoordIdx;
}

inline const MC2BoundingBox&
CoordsArg::getBBox() const
{
   return m_bbox;
}

#endif // TILE_FEATURE_ARG_H

