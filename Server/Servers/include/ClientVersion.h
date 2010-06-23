/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CLIENTVERSION_H
#define CLIENTVERSION_H

#include "config.h"
#include "MC2String.h"


/**
 * Class for holding a Wayfinder version.
 *
 */
class ClientVersion {
   public:
      /**
       * Constructor.
       *
       * @param ver The version as string.
       * @param programVerOnly If we allow only program version
       */
      ClientVersion( const MC2String& ver, bool programVerOnly = false );

      /**
       * Constructor.
       */
      ClientVersion( uint32 programMajorVersion, 
                     uint32 programMinorVersion, 
                     uint32 programMiniVersion,
                     uint32 resourceMajorVersion = 0, 
                     uint32 resourceMinorVersion = 0, 
                     uint32 resourceMiniVersion = 0,
                     uint32 MLFWVersion = 0 );

      /**
       * Destructor.
       */
      ~ClientVersion();

      /**
       * Less than operator.
       */
      bool operator<( const ClientVersion& o ) const;

      /**
       * Get the programMajorVersion.
       *
       * @return The programMajorVersion.
       */
      uint32 getProgramMajorVersion() const;

      /**
       * Get the programMinorVersion.
       *
       * @return The programMinorVersion.
       */
      uint32 getProgramMinorVersion() const;

      /**
       * Get the programMiniVersion.
       *
       * @return The programMiniVersion.
       */
      uint32 getProgramMiniVersion() const;

      /**
       * Get the resourceMajorVersion.
       *
       * @return The resourceMajorVersion.
       */
      uint32 getResourceMajorVersion() const;

      /**
       * Get the resourceMinorVersion.
       *
       * @return The resourceMinorVersion.
       */
      uint32 getResourceMinorVersion() const;

      /**
       * Get the resourceMiniVersion.
       *
       * @return The resourceMiniVersion.
       */
      uint32 getResourceMiniVersion() const;

      /**
       * Get the MLFWVersion.
       */
      uint32 getMLFWVersion() const;

      /**
       * Get the version as string.
       */
      const MC2String& getVersion() const;

      /**
       * Get program version as string
       */
      MC2String getProgramVersionString() const;

   private:
      /**
       * The version as string.
       */
      MC2String m_ver;

      /**
       * The programMajorVersion.
       */
      uint32 m_programMajorVersion;

      /**
       * The programMinorVersion.
       */
      uint32 m_programMinorVersion;

      /**
       * The programMiniVersion.
       */
      uint32 m_programMiniVersion;

      /**
       * The resourceMajorVersion.
       */
      uint32 m_resourceMajorVersion;

      /**
       * The resourceMinorVersion.
       */
      uint32 m_resourceMinorVersion;

      /**
       * The resourceMiniVersion.
       */
      uint32 m_resourceMiniVersion;

      /**
       * The MLFWVersion.
       */
      uint32 m_MLFWVersion;
};


#endif // CLIENTVERSION_H

