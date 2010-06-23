/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTERNAL_SEARCH_DESC_H
#define EXTERNAL_SEARCH_DESC_H

#include "config.h"
#include "ExternalSearchConsts.h"

#include "MC2String.h"

#include <map>

class LangType;
class ExternalSearchDescEntry;
class Packet;
class DataBuffer;

/**
 *   Objects describing the fields of a search request.
 *   Created using ExternalSearchDescGenerator.
 */
class ExternalSearchDesc : public ExternalSearchConsts {   
public:

   /**
    *   Entry for one icon for the service.
    */
   class IconEntry {
   public:
      /// Creates new IconEntry
      IconEntry( const MC2String& name,
                 const MC2String& clientID,
                 int xsize,
                 int ysize )
            : m_name(name),
              m_clientID( clientID ),
              m_xsize(xsize),
              m_ysize(ysize) {}
      /// Returns x size
      int getXSize() const { return m_xsize; }
      /// Returns y size
      int getYSize() const { return m_ysize; }
      /// Returns the image name
      const MC2String& getIconName() const { return m_name; }
      /// Returns the client id
      const MC2String& getClientID() const { return m_clientID; }
      /// Prints the stuff
      friend ostream& operator<<(ostream& o, const IconEntry& icon ) {
         return o << MC2CITE(icon.m_name)
                  << ", " << MC2CITE(icon.m_clientID)
                  << ", " << icon.m_xsize
                  << ", " << icon.m_ysize;
      }
   private:
      MC2String m_name;
      MC2String m_clientID;
      int m_xsize;
      int m_ysize;
   };
   
   /// Type of vector to store the entries in.
   typedef vector<ExternalSearchDescEntry> entryvector_t;

   /// Type of vector to store the icons in.
   typedef vector<IconEntry> iconList_t;

   /// Map of values sent from client.
   typedef map<int, MC2String> valmap_t;
   
   /// Checks if the params are ok.
   bool checkParams( const valmap_t& value_map,
                     uint32 service ) const;

   /// Returns the service of this ExternalSearchDesc
   uint32 getService() const;
   /// Returns the language of this ExternalSearchDesc
   LangType getLang() const;
   /// Sets the service and updates the icons.
   void setService( uint32 serviceId );
   /// Sets the language
   void setLang( const LangType& langType );
   /// Sets the name
   void setName( const MC2String& name );
   /// Sets the colour
   void setColour( const uint32 colour );
   /// Gets a reference to the entries
   const entryvector_t& getEntries() const;
   /// Swaps in a new set of entries.
   void swapEntries( entryvector_t& newEntries );
   /// Returns the CRC for the desc.
   uint32 getCRC() const;
   /// Calculates the CRC for the desc
   void calcCRC();
   /// Returns the name of the service
   const MC2String& getName() const;
   /// Returns the color for the service
   uint32 getColour() const;
   /// Returns true if the desc is empty
   bool empty() const;

   /// sets field value of top region
   bool setTopRegionField( valmap_t& map, uint32 topRegion ) const;
   /// sets field values of what and where, possibly using categoryName
   bool setWhatWhereField( valmap_t& map, 
                           const MC2String& what,
                           const MC2String& where,
                           const MC2String& categoryName ) const;
   
   uint32 getTopRegionID() const;  
   
   /// Saves the data to a packet.
   int save( Packet* packet, int& pos ) const;

private:


   /// Checks a single parameter.
   bool checkOneParam( int inParamID,
                       const MC2String& inParamVal,
                       const ExternalSearchDescEntry& facit ) const;


   /// For saving and calcing CRC.
   friend class DataBuffer;
   
   /// Service of this desc.
   uint32 m_service;
   /// Language of this desc.
   uint32 m_langType;
   /// Parameter entries
   entryvector_t m_entries;
   /// Name
   MC2String m_name;
   /// Colour
   uint32 m_colour;
   /// CRC
   uint32 m_crc;
   /// Vector of icons
   iconList_t m_icons;
};

#endif
