/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MODULETYPES_H
#define MODULETYPES_H

#include "config.h"

/**
 * A type that identifies a Module.
 * Used by the Server to get the correct ip and port.
 */
enum  moduletype_t{
   /// MODULE_TYPE_INVALID
   MODULE_TYPE_INVALID  = 0,
   /// MODULE_TYPE_MAP
   MODULE_TYPE_MAP      = 1,
   /// MODULE_TYPE_ROUTE
   MODULE_TYPE_ROUTE    = 2,
   /// MODULE_TYPE_SEARCH
   MODULE_TYPE_SEARCH   = 3,
   /// MODULE_TYPE_USER
   MODULE_TYPE_USER     = 4,
   /// MODULE_TYPE_SMS
   MODULE_TYPE_SMS      = 5,
   /// MODULE_TYPE_TEST
   MODULE_TYPE_TEST     = 6,
   /// MODULE_TYPE_SMTP
   MODULE_TYPE_SMTP     = 7,
   /// MODULE_TYPE_TRAFFIC
   MODULE_TYPE_TRAFFIC  = 8,
   /// MODULE_TYPE_GFX
   MODULE_TYPE_GFX      = 9,
   /// ExternalServiceModule
   MODULE_TYPE_EXTSERVICE = 10,
   /// TileModule
   MODULE_TYPE_TILE       = 11,
   /// CommunicationModule
   MODULE_TYPE_COMMUNICATION  = 12,
   /// Max number of modules, used for 'for' statements, etc
   MODULE_TYPE_MAX_MODULE_NUMBER,
};

class ModuleTypes {
public:
   /**
    *    Returns the name of the module.
    */
   static const char* moduleTypeToString( moduletype_t moduleType );

   /**
    *    Returns short name for module. For use in Supervisor.
    */
   static const char* moduleTypeToShortString( moduletype_t moduleType );
   
private:

   struct module_name_entry_t {
      /// Long name is used for printing and reading props
      const char* const longName;
      /// Short name is used in supervisor
      const char* const shortName;
   };
   
   /// Module names
   static const module_name_entry_t c_moduleNames[];
   
};

#endif // MODULETYPES_H
