/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TRAFFICSITUATION_H
#define TRAFFICSITUATION_H

#include "config.h"
#include "MC2String.h"
#include <vector>
#include "TrafficSituationElement.h"
#include "TrafficDataTypes.h"

class TrafficSituation 
{
  public:

   /**
    *  Constructor
    */
   TrafficSituation();

   /**
    *  Constructor
    */
   TrafficSituation(const MC2String& situationReference,
                    const MC2String& locationTable,
                    uint32 version = 0);
   /**
    *  Destructor
    */
   virtual ~TrafficSituation();
   
   // Returns the situation reference
   const MC2String& getSituationReference() const;
   
   // Sets the situation reference
   void setSituationReference(const MC2String& sitRef);

   // Returns the location table
   const MC2String& getLocationTable() const;

   // Sets the location table
   void setLocationTable(const MC2String& table);

   // Returns the version
   uint32 getVersion() const;
   void setVersion( const uint32 version );

   // Returns the TrafficSituation elements
   const vector<TrafficSituationElement*>& getSituationElements() const;
   
   // Adds a TrafficSituationElement
   void addSituationElement(TrafficSituationElement* situationElem);

   // Returns the number of elements
   uint32 getNbrElements() const;
   
   
  private:
   // Member variables   
   // The situation reference
   MC2String m_situationReference;
   
   // The location table
   MC2String m_locationTable;

   // The version of the traffic situation
   uint32 m_version;
   
   // TrafficSituation elements
   vector<TrafficSituationElement*> m_situationElements;
};

#endif

