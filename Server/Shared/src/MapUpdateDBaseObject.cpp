/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "MapUpdateDBaseObject.h"

const uint32 MapUpdateDBaseObject::m_nbrFields = 11;

const char* MapUpdateDBaseObject::m_fieldNames[ m_nbrFields ] = { 
   "id",
   "type",
   "insertionTime",
   "lat",
   "lon",
   "originator",
   "description",
   "originatorUserID",
   "status",
   "handledBy", 
   "action"
};

const DBaseElement::element_t 
MapUpdateDBaseObject::m_fieldTypes[ m_nbrFields ] = {
      DBaseElement::DB_INT_NOTNULL,           // id
      DBaseElement::DB_INT_NOTNULL,           // type
      DBaseElement::DB_INT_NOTNULL,           // insertionTime
      DBaseElement::DB_INT_NOTNULL,           // lat
      DBaseElement::DB_INT_NOTNULL,           // lon
      DBaseElement::DB_LARGESTRING_NOTNULL,   // originator
      DBaseElement::DB_LARGESTRING_NOTNULL,   // description
      DBaseElement::DB_INT_NOTNULL,           // originatorUserID
      DBaseElement::DB_INT,                   // status
      DBaseElement::DB_LARGESTRING,           // handledBy
      DBaseElement::DB_LARGESTRING            // action
};

const char* 
MapUpdateDBaseObject::m_fieldDescriptionStrs[ m_nbrFields ] = {
   "id",
   "type",
   "insertionTime",
   "lat",
   "lon",
   "originator",
   "description",
   "originatorUserID",
   "status",
   "handledBy", 
   "action"
};

const char* 
MapUpdateDBaseObject::m_tableStr = "ISABMapUpdates";

const StringTable::stringCode 
MapUpdateDBaseObject::m_fieldDescriptions[ m_nbrFields ] = {
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING,
      StringTable::NOSTRING 
};



MapUpdateDBaseObject::MapUpdateDBaseObject()
    : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields );
}

MapUpdateDBaseObject::MapUpdateDBaseObject(const MapUpdateDBaseObject& copy )
    : DBaseObject( copy )
{
   // Do not call init from copy constructor
}

MapUpdateDBaseObject::MapUpdateDBaseObject(SQLQuery* pQuery )
      : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields, pQuery );
}


MapUpdateDBaseObject::MapUpdateDBaseObject( const Packet* packet, int& pos )
   : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields, packet, pos );
}

MapUpdateDBaseObject::MapUpdateDBaseObject(uint32 type, 
                                           uint32 time,
                                           int32 lat, 
                                           int32 lon,
                                           const char* description,
                                           const char* originator,
                                           uint32 loginID)
   : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields);

   // Add data to the fields
   setInt(field_ID, 0);
   setInt(field_type, type);
   setInt(field_insertionTime, time);
   setInt(field_lat, lat);
   setInt(field_lon, lon);
   setString(field_originator, originator);
   setString(field_description, description);
   setInt(field_originatorUserID, loginID);
   //setInt(field_status, 0);
   //setString(field_handledBy, "");
   //setString(field_action, "");
}


MapUpdateDBaseObject::MapUpdateDBaseObject(uint32 ID,
                                           uint32 status,
                                           const char* handledBy,
                                           const char* action)
   : DBaseObject( m_nbrFields )
{
   // Always call init from subclass
   init( m_nbrFields);

   // Add data to the fields
   setInt(field_ID, ID);
   setInt(field_status, status);
   setString(field_handledBy, handledBy);
   setString(field_action, action);
}

MapUpdateDBaseObject::~MapUpdateDBaseObject() 
{
   // Nothing to do
}


const char*
MapUpdateDBaseObject::getTableStr() const 
{
   return m_tableStr;
}


const char** 
MapUpdateDBaseObject::getFieldDescriptions() const 
{
   return m_fieldDescriptionStrs;
}


const StringTable::stringCode* 
MapUpdateDBaseObject::getFieldStringCode() const 
{
   return m_fieldDescriptions;
}

/*
bool 
MapUpdateDBaseObject::setAddMapUpdate(uint32 type, 
                                      uint32 time,
                                      int32 lat, 
                                      int32 lon,
                                      const char* description,
                                      const char* originator,
                                      uint32 loginID)
{
   setInt(field_ID, 0);
   setInt(field_type, type);
   setInt(field_insertionTime, time);
   setInt(field_lat, lat);
   setInt(field_lon, lon);
   setString(field_originator, originator);
   setString(field_description, description);
   setInt(field_originatorUserID, loginID);
   setInt(field_status, 0);
   setString(field_handledBy, "");
   setString(field_action, "");
   return (true);
}
*/

/*
bool 
MapUpdateDBaseObject::setMapUpdateStatus( uint32 ID,
                                          uint32 status,
                                          const char* handledBy,
                                          const char* action)
{

   return (true);
}
*/

const char**
MapUpdateDBaseObject::getFieldNames() const 
{
   return m_fieldNames;
}


const DBaseElement::element_t* 
MapUpdateDBaseObject::getFieldTypes() const 
{
   return m_fieldTypes;
}







