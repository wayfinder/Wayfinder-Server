/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "TrafficSituation.h"


TrafficSituation::TrafficSituation() :
   m_situationReference(),
   m_locationTable(),
   m_version( 0 )
{
}

TrafficSituation::TrafficSituation(const MC2String& situationReference,
                                   const MC2String& locationTable,
                                   uint32 version) :
   m_situationReference( situationReference ),
   m_locationTable( locationTable ),
   m_version( version )
{
}

TrafficSituation::~TrafficSituation()
{
   vector<TrafficSituationElement*>::iterator it;
   for(it = m_situationElements.begin(); it != m_situationElements.end();
       it++)
   {
      TrafficSituationElement* element = *it;
      delete element;
   }
   m_situationElements.clear();
}

const MC2String&
TrafficSituation::getSituationReference() const
{
   return m_situationReference;
}

void TrafficSituation::setSituationReference(const MC2String& sitRef)
{
   m_situationReference = sitRef;
}

const MC2String&
TrafficSituation::getLocationTable() const
{
   return m_locationTable;
}

void
TrafficSituation::setLocationTable(const MC2String& table)
{
   m_locationTable = table;
}

uint32
TrafficSituation::getVersion() const
{
   return m_version;
}

void
TrafficSituation::setVersion( const uint32 version)
{
   m_version = version;
}


const vector<TrafficSituationElement*>&
TrafficSituation::getSituationElements() const
{
   return m_situationElements;
}
                     
void
TrafficSituation::addSituationElement(TrafficSituationElement* situationElem)
{
   m_situationElements.push_back(situationElem);
}

uint32
TrafficSituation::getNbrElements() const
{
   return m_situationElements.size();
}

