/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef POIINFO_H
#define POIINFO_H

#include "config.h"
#include "MC2String.h"
#include "LangTypes.h"
#include "Array.h"

#include <vector>
#include <set>
#include <iosfwd>

class DataBuffer;


/**
 * Holds information about one specific information type 
 * for a POI.
 */
class POIInfoData {
public:
   /// the type of the information
   typedef uint32 type_t;

   /**
    * @param type information type
    * @param lang language of the information
    * @param info the information as a string
    */
   POIInfoData( type_t type, LangType::language_t lang,
                const MC2String& info ):
      m_type( type ),
      m_lang( lang ),
      m_info( info )
   { }

   /// @return the type of information
   type_t getType() const { return m_type; }
   /// @return the language of the information
   LangTypes::language_t getLang() const { return m_lang; }
   /// @return the information as a string
   const MC2String& getInfo() const { return m_info; }

private:
   type_t m_type; ///< what kind of type this information is
   LangTypes::language_t m_lang; ///< language of the info
   MC2String m_info; ///< information such as "murvstreet"
};


/**
 * Contains a set of informations about a POI 
 * which can be loaded from a file.
 */
class POIInfo {
public:
   /// data type set for each poi
   typedef set<POIInfoData::type_t> DataTypeSet;
   typedef STLUtility::Array<const POIInfoData> InfoArray;

   POIInfo();

   /**
    * @param poiInfoSize hints for reserving poiinfo size
    */
   POIInfo( uint32 staticID, bool dynamicInfo,
            uint32 poiInfoSize );

   /**
    * Clone a POIInfo but only clone the data types
    * that the client are interested in which is described in
    * the set keySet.
    * @param copyThis the info to copy
    * @param keySet the data types that should be copied.
    */
   explicit POIInfo( const POIInfo& copyThis,
                     const DataTypeSet& keySet = DataTypeSet() );

   ~POIInfo();

   /** 
    * Load POI information from a file.
    * This will load all static data. The dynamic data has to 
    * be loaded seperatly if needed ( the holdsDynamicInfo()
    * indicates if it holds dynamic data in a database ).
    * 
    * @param filename the file to load from
    * @param offset the offset in the file
    * @return true of information was loaded successfully
    */
   bool load( const MC2String& filename, uint32 offset );

   /**
    * Load POI Information from a databuffer. See load from file above.
    * @param buff the buffer to load from
    */
   void load( DataBuffer& buff );

   /**
    * Adds new information to the set. Used to add dynamic information.
    * @param info the information to be added, the information will
    *        be deleted by this instance.
    */
   void addInfo( POIInfoData* info ) { m_poiInfos.push_back( info ); }
   /// @return the information
   const vector<POIInfoData*>& getInfos() const { return m_poiInfos; }
   /**
    * @param type the data type to search for
    * @return poi info data
    */
   InfoArray findData( POIInfoData::type_t type ) const;

   /// @return static id of the POI
   uint32 getStaticID() const { return m_staticID; }
   /// @return true if the poi contains dynamic information
   bool hasDynamicInfo() const { return m_dynamicInfo; }

private:
   uint32 m_staticID; //< static id
   bool m_dynamicInfo; //< whether it contains dynamic info
   typedef vector< POIInfoData* > InfoDataVector;
   InfoDataVector m_poiInfos; //< all the information about this item
};

/// prints information about an info, for debug purposes
ostream& operator << ( ostream& ostr, const POIInfo& info );
/// prints information about an info, for debug purposes
ostream& operator << ( ostream& ostr, const POIInfoData& info );

#endif // POIINFO_H
