/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TILE_IMPORTANCE_TABLE_H
#define TILE_IMPORTANCE_TABLE_H

#include "TileMapConfig.h"
#include <map>
#include <vector>


class BitBuffer;

/**
 *    TileImportanceNotice.
 *
 *    Used to know at what importance a certain feature should
 *    be included.
 *    
 */
class TileImportanceNotice {
   
   public:
      
      /**
       *    Constructor.
       *    @param   detailLevel The detail level for feature.
       *    @param   type        The type of feature the notice applies to.
       *    @param   threshold   Optional parameter, 
       *                         default set to MAX_UINT32.
       *                         If specified, the number of squarepixels
       *                         of the feature that must be present on
       *                         the screen before including the feature.
       */
      TileImportanceNotice( float64 detailLevel,
                            uint16 maxScale,
                            uint16 type,
                            uint32 threshold = MAX_UINT32 );
      
      /**
       *    Empty constructor.
       */
      TileImportanceNotice() {}

      /**
       *    Load from buffer.
       *    @param   buf   The buffer.
       */
      void load( BitBuffer& buf );

      /**
       *    Save from buffer.
       *    @param   buf   The buffer.
       */
      void save( BitBuffer& buf ) const;
        
      /**
       *    Dump all data to the stream.
       *    @param   stream   The stream.
       */
      void dump( ostream& stream ) const;

      /**
       *    @return  Detaillevel.
       */
      inline float64 getDetailLevel() const;

      /**
       *    @return  Importancelevel
       */
      inline int getImportance() const;

      /**
       *    @return  Max scale.
       */
      inline uint16 getMaxScale() const;

      /**
       *    @return  Feature type.
       */
      inline uint16 getType() const;

      

      /**
       *    @return Threshold. The number of squarepixels on the screen 
       *    that the feature area corresponds to before 
       *    including the feature. If set to MAX_UINT32 the area should 
       *    be disregarded.
       */
      inline uint32 getThreshold() const; 

      /**
       *    @return True if the importance is present on a fixed scale.
       */
      inline bool fixedScale() const;
      
   private:      
      /**
       *    The detaillevel that the feature should be shown on.
       */
      float64 m_detailLevel;
      
      /**
       *    The importance.
       */
      int m_importance;

      /**
       *    The max scale.
       */
      uint16 m_maxScale;

      /**
       *    The type of feature.
       */
      uint16 m_type;

      /**
       *    The number of squarepixels on the screen that the feature
       *    area corresponds to before including the feature.
       *    If set to MAX_UINT32 the area is disregarded.
       */
      uint32 m_threshold;

};

// Forward
class ServerTileMapFormatDesc;

/**
 *    Table of TileImportanceNotices.
 *    
 */
class TileImportanceTable : private multimap<uint16, TileImportanceNotice> {
   
   public:
      /**
       *    Assignment operator that is not implemented so that it
       *    won't link if being used.
       */
      TileImportanceTable& operator = ( const TileImportanceTable& other );

      /**
       *    Copy constructor that is not implemented so that it
       *    won't link if being used.
       */
      TileImportanceTable( const TileImportanceTable& other );
      
      /**
       *    Empty constructor.
       */
      TileImportanceTable() {};

      /**
       *    The ServerTileMapFormatDesc needs to be able to construct 
       *    the importance table.
       */
      friend class ServerTileMapFormatDesc;
      
      /**
       *    Destructor.
       */
      ~TileImportanceTable();
      
      /**
       *    Load from buffer.
       *    @param   buf   The buffer.
       */
      void load( BitBuffer& buf );

      /**
       *    Save from buffer.
       *    @param   buf   The buffer.
       */
      void save( BitBuffer& buf ) const;
      
      /**
       *    Get the number of importances present for the current
       *    scalelevel and detaillevel.
       *    @param   scale       The scalelevel.
       *    @param   detailLevel The detaillevel.
       *    @return  The number of importances.
       */
      int getNbrImportanceNbrs( uint16 scale, int detailLevel ) const;

      /**
       *    Fills the vector with scales where something happens.
       *    Used when getting all the parameters for an area.
       */
      void getInterestingScales( vector<uint32>& scales ) const;
      
      /**
       *    Get the TileImportanceNotice for the specified importance
       *    level and detaillevel.
       *    @param   importanceLevel   The importance level.
       *    @param   detailLevel       The detail level.
       *    @return  The corresponding TileImportanceNotice.
       */
      const TileImportanceNotice* getImportanceNbr( int importanceLevel,
                                                    int detailLevel ) const;

      /**
       *    Get the first TileImportanceNotice of the specified type.
       *    @param   type  The type.
       *    @return  The corresponding TileImportanceNotice or NULL
       *             if not found.
       */
      const TileImportanceNotice* getFirstOfType( uint16 type ) const;
      
      /**
       *    Dump the table to the stream.
       *    @param   stream   The stream.
       */
      void dump( ostream& stream ) const;
      inline bool isValidImportanceNbr( int importanceNbr,
                                        int detailLevel ) const;
   private:
      /**
       *    Get the TileImportanceNotice for the specified importance
       *    level and detaillevel.
       *    @param   importanceLevel   The importance level.
       *    @param   detailLevel       The detail level.
       *    @return  The corresponding TileImportanceNotice.
       */
      const TileImportanceNotice* getImportanceNbrSlow( 
                                             int importanceLevel,
                                             int detailLevel ) const;
      
      /**
       *    Build the importance matrix.
       */
      void buildMatrix();
      
      /**
       *    First index is detaillevel. Second is importance nbr.
       *    I.e. m_importanceMatrix[ detaillevel ][ importanceNbr ]
       */
      vector< vector< const TileImportanceNotice* >* > m_importanceMatrix;
      
};


// --- Implementation of inlined methods. ---

inline float64 
TileImportanceNotice::getDetailLevel() const
{
   return m_detailLevel;
}

inline int 
TileImportanceNotice::getImportance() const
{
   return m_importance;
}

inline uint16 
TileImportanceNotice::getMaxScale() const
{
   return m_maxScale;
}

inline uint16 TileImportanceNotice::getType() const
{
   return m_type;
}
   
inline uint32 TileImportanceNotice::getThreshold() const
{
   return m_threshold;
}
      
inline bool 
TileImportanceNotice::fixedScale() const
{
   return getThreshold() == MAX_UINT32;
}

inline bool TileImportanceTable::
isValidImportanceNbr( int importanceNbr,
                      int detailLevel ) const {
   if ( importanceNbr < 0 ||
        detailLevel < 0 ) {
      return false;
   }

   // must have a valid detail level too
   if ( static_cast<uint32>( detailLevel ) < m_importanceMatrix.size() ) {
      return static_cast< uint32 >( importanceNbr ) < 
         (*m_importanceMatrix[ detailLevel ] ).size();
   }
   return false;
}

#endif

