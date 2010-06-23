/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVADDRESS_H
#define NAVADDRESS_H

#include "config.h"

/**
 *    Class that contains an address and type of a navigator.
 */
class NavAddress {

public:

   /**
    *   Address types.
    *   ADDRESS_TYPE_NULL is invalid.
    *   ADDRESS_TYPE_ISABNAVIGATOR is for our Navigator.
    */
   enum NavAddressType { ADDRESS_TYPE_NULL,
                         ADDRESS_TYPE_ISABBOX};

   /**
    *   Creates a new INVALID address.
    */
   NavAddress();
   
   /**
    *   Create a new address.
    *   @param type    The type of navigatoraddress.
    *   @param address The address string. 
    */
   NavAddress( NavAddressType type,
               const char* address);
               
   /**
    *   Copy-constructor.
    */
   NavAddress( const NavAddress& na);

   /**
    *   Assignment operator.
    *   @param na The address to be copied.
    *   @return The copy.
    */
   const NavAddress& operator=(const NavAddress& na);
   
   /**
    *   Destructor.
    */
   virtual ~NavAddress();
   
   /**
    *   @return The type of this address.
    */
   inline NavAddressType getType() const;
   
   /**
    *   @return The address string of this address. (Phonenumber)
    */
   inline const char* getString() const;

protected:

   /**
    *   Fill in the stuff we need to copy in constructors and operator=.
    */
   void fillin( NavAddressType type,
                const char* address);
   
   /**
    *   The address type.
    */
   NavAddressType m_type;
   
   /**
    *   The string part of the address.
    */
   char* m_string;
   
};


/* Implementation of inlined methods */

inline NavAddress::NavAddressType
NavAddress::getType() const {
   return m_type;
}

inline const char*
NavAddress::getString() const {
   return m_string;
} 

#endif
