/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ClientVersion.h"
#include "MC2Exception.h"

#include <stdlib.h>

#define VER_FORMAT "%u.%u.%u:%u.%u.%u:%u"
#define PROG_VER_FORMAT "%u.%u.%u"

ClientVersion::ClientVersion( const MC2String& ver, bool programVerOnly ) {
   bool incorrectFormat = false;

   if ( ver.empty() ) {
      m_programMajorVersion = 0;
      m_programMinorVersion = 0;
      m_programMiniVersion = 0;
      m_resourceMajorVersion = 0;
      m_resourceMinorVersion = 0;
      m_resourceMiniVersion = 0;
      m_MLFWVersion = 0;
   } else if( programVerOnly ) {
      
      m_resourceMajorVersion = 0;
      m_resourceMinorVersion = 0;
      m_resourceMiniVersion = 0;
      m_MLFWVersion = 0;

      incorrectFormat = sscanf( ver.c_str(), PROG_VER_FORMAT, 
                                &m_programMajorVersion, 
                                &m_programMinorVersion, 
                                &m_programMiniVersion ) != 3;
      
   } else { 
      incorrectFormat = sscanf( ver.c_str(), VER_FORMAT, 
                                &m_programMajorVersion, 
                                &m_programMinorVersion, 
                                &m_programMiniVersion, 
                                &m_resourceMajorVersion, 
                                &m_resourceMinorVersion, 
                                &m_resourceMiniVersion, 
                                &m_MLFWVersion ) != 7;
   }

   if ( incorrectFormat ) {
      MC2String errorStr( "ClientVersion bad version \"" + ver + "\"" );
      mc2log << error << errorStr << endl;
      
      throw MC2Exception(  errorStr );
   }

   m_ver = ver;
}

ClientVersion::ClientVersion( uint32 programMajorVersion, 
                              uint32 programMinorVersion, 
                              uint32 programMiniVersion,
                              uint32 resourceMajorVersion, 
                              uint32 resourceMinorVersion, 
                              uint32 resourceMiniVersion,
                              uint32 MLFWVersion )
      : m_programMajorVersion( programMajorVersion ),
        m_programMinorVersion( programMinorVersion ),
        m_programMiniVersion( programMiniVersion ),
        m_resourceMajorVersion( resourceMajorVersion ),
        m_resourceMinorVersion( resourceMinorVersion ),
        m_resourceMiniVersion( resourceMiniVersion ),
        m_MLFWVersion( MLFWVersion )
{
   if ( programMajorVersion == 0 && programMinorVersion == 0 ) {
      m_ver = "";
   } else {
      char tmpStr[ 70 + 7 + 1 ];
      sprintf( tmpStr, VER_FORMAT, 
               m_programMajorVersion, 
               m_programMinorVersion, 
               m_programMiniVersion, 
               m_resourceMajorVersion, 
               m_resourceMinorVersion, 
               m_resourceMiniVersion, 
               m_MLFWVersion );
      m_ver = tmpStr;
   }
}

ClientVersion::~ClientVersion() {
}

bool
ClientVersion::operator<( const ClientVersion& o ) const {
   if ( getProgramMajorVersion() != o.getProgramMajorVersion() ) {
      return getProgramMajorVersion() < o.getProgramMajorVersion();
   } else if ( getProgramMinorVersion() != o.getProgramMinorVersion() ) {
      return getProgramMinorVersion() < o.getProgramMinorVersion();
   } else {
      return getProgramMiniVersion() < o.getProgramMiniVersion();
   }
}

uint32
ClientVersion::getProgramMajorVersion() const {
   return m_programMajorVersion;
}

uint32
ClientVersion::getProgramMinorVersion() const {
   return m_programMinorVersion;
}

uint32
ClientVersion::getProgramMiniVersion() const {
   return m_programMiniVersion;
}

uint32
ClientVersion::getResourceMajorVersion() const {
   return m_resourceMajorVersion;
}

uint32
ClientVersion::getResourceMinorVersion() const {
   return m_resourceMinorVersion;
}

uint32
ClientVersion::getResourceMiniVersion() const {
   return m_resourceMiniVersion;
}

uint32
ClientVersion::getMLFWVersion() const {
   return m_MLFWVersion;
}

const MC2String&
ClientVersion::getVersion() const {
   return m_ver;
}

MC2String 
ClientVersion::getProgramVersionString() const {
   size_t pos = m_ver.find(':');
   if ( pos == MC2String::npos ) {
      return m_ver;
   } else {
      return m_ver.substr(0, pos);
   }
}
