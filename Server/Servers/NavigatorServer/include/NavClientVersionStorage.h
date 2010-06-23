/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef NAVCLIENTVERSIONSTORAGE_H
#define NAVCLIENTVERSIONSTORAGE_H

#include "config.h"
#include <set>


/**
 * Class holding a specific client type's versions.
 *
 */
class NavClientVersion {
   public: 
      /**
       * Constructor.
       *
       * @param clientType The clientType, is copied.
       * @param clientTypeOptions The clientTypeOptions, is copied.
       * @param programMajorVersion The major program version.
       * @param programMinorVersion The minor program version.
       * @param programMiniVersion The mini program version.
       * @param resourceMajorVersion The major resource version.
       * @param resourceMinorVersion The minor resource version.
       * @param resourceMiniVersion The mini resource version.
       */
      NavClientVersion( const char* clientType, 
                        const char* clientTypeOptions,
                        uint32 programMajorVersion, 
                        uint32 programMinorVersion, 
                        uint32 programMiniVersion,
                        uint32 resourceMajorVersion, 
                        uint32 resourceMinorVersion, 
                        uint32 resourceMiniVersion );


      /**
       * Destructor.
       */
      ~NavClientVersion();


      /**
       * Get the clientType.
       * 
       * @return The clientType.
       */
      const char* getClientType() const;


      /**
       * Get the clientTypeOptions.
       * 
       * @return The clientTypeOptions.
       */
      const char* getClientTypeOptions() const;


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


   private:
      /**
       * The clientType.
       */
      char* m_clientType;


      /**
       * The clientTypeOptions
       */
      char* m_clientTypeOptions;


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
       * NavClientVersionStorage may use m_clientType as it likes.
       */
      friend class NavClientVersionStorage;
};


/**
 * Class for comparing two NavClientVersion.
 *
 */
class NavClientVersionComp {
   public:
      /**
       * Less than compare two NavClientVersion.
       */
      bool operator() ( const NavClientVersion* a,
                        const NavClientVersion* b ) const;
};


/**
 * Class for holding a number of program and resource versions for clients.
 * 
 */
class NavClientVersionStorage {
   public:
      /**
       * Constructor.
       */
      NavClientVersionStorage();


      /**
       * Destructor.
       */
      virtual ~NavClientVersionStorage();


      /**
       * Get the program and resource versions for a specific client type.
       * 
       * @param clientType The client type.
       * @return Returns NULL if no match.
       */
      const NavClientVersion* getVersion( const char* clientType ) const;
      

      /**
       * Add a NavClientVersion.
       *
       * @param ver The NavClientVersion to add, is owned by this class 
       *            now.
       */
      void addVersion( NavClientVersion* ver );


   private:
      /**
       * A set of NavClientVersions.
       */
      typedef set< NavClientVersion*, NavClientVersionComp > navClientSet;


      /**
       * The client versions.
       */
      navClientSet m_clientVersions;


      /**
       * The NavClientVersion used in searches.
       */
      NavClientVersion m_navClientVersion;
};


#endif // NAVCLIENTVERSIONSTORAGE_H

