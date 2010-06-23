/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef EXTERNAL_SEARCH_DESC_HENERRAGOT_H
#define EXTERNAL_SEARCH_DESC_HENERRAGOT_H

#include "config.h"

#include <map>
#include <vector>

class ExtService;
class LangType;
class ExternalSearchDesc;

/**
 * Generates external service descriptions.
 * This class is not thread safe.
 */
class ExternalSearchDescGenerator {
public:

   /// Constructor
   ExternalSearchDescGenerator();

   /// Destructor
   ~ExternalSearchDescGenerator();

   /// Returns the params for the requested service and language.
   const ExternalSearchDesc* getDescPtr( const ExtService& service,
                                         const LangType& lang ) const;
   
   /// Returns the params for the requested service and language.
   const ExternalSearchDesc& getDesc( const ExtService& service,
                                      const LangType& lang ) const;

   /// Puts all descs in a vector. 
   int getDescs( vector<const ExternalSearchDesc*>& descs,
                 const LangType& lang ) const;

   /// Returns the crc of all the services for the requested lang.
   uint32 getCRC( const LangType& lang ) const;

private:

   
   /// Creates a new desc from the service and language.
   int makeDesc( const ExtService& service,
                 const LangType& langType );

   /** maps a <code>pair<service ID, language type > </code> to 
    *  an ExternalSearchDesc descriptor.
    */
   typedef map< pair<int,int>, ExternalSearchDesc*> serviceMap_t;
   
   /// Map of services with servicenumber in first and lang in second.
   serviceMap_t m_serviceMap;

};

#endif
