/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "NavClientVersionStorage.h"
#include "StringUtility.h"


/* --- NavClientVersion ----------------------------------------------- */


NavClientVersion::NavClientVersion( const char* clientType, 
                                    const char* clientTypeOptions,
                                    uint32 programMajorVersion, 
                                    uint32 programMinorVersion, 
                                    uint32 programMiniVersion,
                                    uint32 resourceMajorVersion, 
                                    uint32 resourceMinorVersion, 
                                    uint32 resourceMiniVersion )
      : m_clientType( StringUtility::newStrDup( clientType ) ),
        m_clientTypeOptions( StringUtility::newStrDup( 
                                clientTypeOptions ) ),
        m_programMajorVersion( programMajorVersion ),
        m_programMinorVersion( programMinorVersion ),
        m_programMiniVersion( programMiniVersion ),
        m_resourceMajorVersion( resourceMajorVersion ),
        m_resourceMinorVersion( resourceMinorVersion ),
        m_resourceMiniVersion( resourceMiniVersion )
{
}


NavClientVersion::~NavClientVersion() {
   delete [] m_clientType;
   delete [] m_clientTypeOptions;
}


const char*
NavClientVersion::getClientType() const {
   return m_clientType;
}


const char*
NavClientVersion::getClientTypeOptions() const {
   return m_clientTypeOptions;
}


uint32
NavClientVersion::getProgramMajorVersion() const {
   return m_programMajorVersion;
}


uint32
NavClientVersion::getProgramMinorVersion() const {
   return m_programMinorVersion;
}


uint32
NavClientVersion::getProgramMiniVersion() const {
   return m_programMiniVersion;
}


uint32
NavClientVersion::getResourceMajorVersion() const {
   return m_resourceMajorVersion;
}


uint32
NavClientVersion::getResourceMinorVersion() const {
   return m_resourceMinorVersion;
}


uint32
NavClientVersion::getResourceMiniVersion() const {
   return m_resourceMiniVersion;
}


/* --- NavClientVersionComp -------------------------------------------- */


bool
NavClientVersionComp::operator() ( const NavClientVersion* a,
                                   const NavClientVersion* b ) const
{
   return StringUtility::strcmp( a->getClientType(), 
                                 b->getClientType() ) < 0;
}


/* --- NavClientVersionStorage ----------------------------------------- */


NavClientVersionStorage::NavClientVersionStorage() 
      : m_navClientVersion( "", "", 0, 0, 0, 0, 0, 0 )
{
   delete [] m_navClientVersion.m_clientType;
   m_navClientVersion.m_clientType = NULL;
}


NavClientVersionStorage::~NavClientVersionStorage() {
   // delete all NavClientVersion
   for ( navClientSet::iterator it = m_clientVersions.begin() ;
         it != m_clientVersions.end() ; ++it )
   {
      delete *it;
   }
}


const NavClientVersion*
NavClientVersionStorage::getVersion( const char* clientType ) const
{
   const_cast<NavClientVersion&>( m_navClientVersion ).m_clientType =
      const_cast<char*>( clientType );
   navClientSet::const_iterator findIt = m_clientVersions.find( 
      const_cast<NavClientVersion*>( &m_navClientVersion ) );
   const_cast<NavClientVersion&>( m_navClientVersion ).m_clientType = NULL;
   
   if ( findIt != m_clientVersions.end() ) {
      return (*findIt);
   } else {
      return NULL;
   }
}


void 
NavClientVersionStorage::addVersion( NavClientVersion* ver ) {
   m_clientVersions.insert( ver );  
}
