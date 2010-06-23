/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MAPUPDATEDBASEOBJECT_H
#define MAPUPDATEDBASEOBJECT_H

#include "config.h"
#include "DBaseObject.h"

/**
 *    Handles the table with the map-updates.
 *
 */
class MapUpdateDBaseObject : public DBaseObject {
   public:
      /**
       *    Create an empty MapUpdateDBaseObject.
       */
      MapUpdateDBaseObject();

      /**
       *    Copy constructor.
       *    @param copy    The object that this should be a copy of.
       */
      MapUpdateDBaseObject( const MapUpdateDBaseObject& copy );

      /**
       *    Reads from SQL database.
       *    @param pDBase
       *    @param num
       */
      MapUpdateDBaseObject( SQLQuery* pQuery );
      
      /**
       *    Reads from Packet.
       *    @param pPacket    The packet where this object is stored.
       *    @param pos        The position in the packet that should
       *                      be used.
       */
      MapUpdateDBaseObject( const Packet* packet, int& pos );

      MapUpdateDBaseObject(uint32 type, 
                           uint32 time,
                           int32 lat, 
                           int32 lon,
                           const char* description,
                           const char* originator,
                           uint32 loginID);

      MapUpdateDBaseObject(uint32 ID,
                           uint32 status,
                           const char* handledBy,
                           const char* action);


      
      /**
       *    Delete this object.
       */
      virtual ~MapUpdateDBaseObject();

      /**
       *    The table's sql name.
       */
      virtual const char* getTableStr() const;

      /**
       *    The descriptions of the fields in ascii.
       */
      virtual const char** getFieldDescriptions() const;

      /**
       *    The descriptions of the fields in stringCodes.
       */
      virtual const StringTable::stringCode* getFieldStringCode() const;

      /**
       *    Add data about a new map-update to this object.
       *    @param type
       *    @param time
       *    @param lat
       *    @param lon
       *    @param description
       *    @param originator
       *    @param loginID
       *    @return True if the data is added, false otherwise.
       */
      /*
      bool setAddMapUpdate(uint32 type, 
                           uint32 time,
                           int32 lat, 
                           int32 lon,
                           const char* description,
                           const char* originator,
                           uint32 loginID);

      bool setMapUpdateStatus(uint32 ID,
                              uint32 status,
                              const char* handledBy,
                              const char* action);
      */

   protected:
      /**
       *    Table field names.
       */
      virtual const char** getFieldNames() const;
      
      /**
       *    Type of elemtnts.
       */
      virtual const DBaseElement::element_t* getFieldTypes() const;


   private:
      /**
       *    The number of fields.
       */
      static const uint32 m_nbrFields;

      /**
       *    The SQL string of the name of the table.
       */
      static const char* m_tableStr;

      /**
       *    The fieldNames
       */
      static const char* m_fieldNames[];

      /**
       *    The fieldTypes.
       */
      static const DBaseElement::element_t m_fieldTypes[];
      
      /**
       *    The fieldDescriptions.
       */
      static const char* m_fieldDescriptionStrs[];

      /**
       *    The descriptions in stringCodes.
       */
      static const StringTable::stringCode m_fieldDescriptions[];

      /**
       *    The field to index enum.
       */
      enum fieldNbr {
         field_ID = 0,
         field_type,
         field_insertionTime,
         field_lat,
         field_lon,
         field_originator,
         field_description,
         field_originatorUserID,
         field_status,
         field_handledBy,
         field_action
      };

};

#endif

