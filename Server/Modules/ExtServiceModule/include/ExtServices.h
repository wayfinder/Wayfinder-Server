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

#ifndef EXTSERVICES_H
#define EXTSERVICES_H

class ExtService {
public:
   enum service_t {
      /// no external service (mc2) Must be 0
      not_external     = 0,
      /// Google Local Search
      google_local_search = 1,
      /// Qype
      qype = 2,
      /// Advertisement server
      adserver = 3,

      /// Number of services. Should be last.
      nbr_services,
   };

   /// Creates a new ExtService that defaults to no_service
   inline ExtService( ) : m_service( not_external ) {}
   
   /// Creates new service.
   inline ExtService( service_t service ) : m_service( service ) {}

   /// Creates new service
   inline ExtService( uint32 serviceId ) {
      if ( serviceId >= nbr_services || serviceId < 0) {
         m_service = not_external;
      } else {
         m_service = static_cast< service_t >(serviceId);
      }
   }

   /// Returns the service as a service_t.
   inline operator service_t() const { return m_service; }
   
private:
   /// The service of this ExtService.
   service_t m_service;
};


#endif // EXTSERVICES_H
