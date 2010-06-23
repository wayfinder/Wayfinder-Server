/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"

#include "OldItem.h"
#include "OldItemIdentifier.h"
#include "StringUtility.h"
#include "GfxConstants.h"

OldItemIdentifier::OldItemIdentifier()
{
   m_name = NULL;
   m_insideItem = NULL;
}


OldItemIdentifier::OldItemIdentifier( const char* name, 
                                ItemTypes::itemType type,
                                int32 lat, 
                                int32 lon )
{
   m_name = NULL;
   m_insideItem = NULL;

   setParameters( name, type, lat, lon );
}
     

OldItemIdentifier::OldItemIdentifier( const char* name, 
                                ItemTypes::itemType type,
                                const char* insideItem )
{
   m_name = NULL;
   m_insideItem = NULL;

   setParameters( name, type, insideItem );
}


OldItemIdentifier::~OldItemIdentifier()
{
   delete m_name;
   delete m_insideItem;
}


void 
OldItemIdentifier::setParameters( const char* name,
                               ItemTypes::itemType type,
                               int32 lat,
                               int32 lon )
{
   delete m_name;
   delete m_insideItem;
   
   m_name = StringUtility::newStrDup( name );
   m_itemType = type;
   m_lat = lat;
   m_lon = lon;
   m_insideItem = NULL;

}


void 
OldItemIdentifier::setParameters( const char* name,
                               ItemTypes::itemType type,
                              const char* insideItem )
{
   delete m_name;
   delete m_insideItem;
   
   m_name = StringUtility::newStrDup( name );
   m_itemType = type;
   m_lat = GfxConstants::IMPOSSIBLE;
   m_lon = GfxConstants::IMPOSSIBLE;
   m_insideItem = StringUtility::newStrDup( insideItem );
}


const uint32 
OldItemIdentifier::maxDistToOpenPolygonInMeters = 10;

