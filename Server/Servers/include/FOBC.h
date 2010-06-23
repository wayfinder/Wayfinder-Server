/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef FOBC_H
#define FOBC_H


#include "config.h"
#include <map>
#include "FOB.h"
#include "ISABThread.h"


/**
 * Class for holding FOBs.
 *
 */
class FOBC {
   public:
      /**
       * Constructor.
       */
      FOBC();


      /**
       * Destructor
       */
      ~FOBC();


      /**
       * Get a file.
       *
       * @param path The file to load.
       * @param forceReRead If to remove any current version of the file.
       *                    Same as removeFile and then getFile.
       */
      aFOB getFile( const MC2String& path, bool forceReRead = false );


      /**
       * Clear the file cache, all entries are removed FOBs are auto 
       * deleted with last reference.
       */
      void clear();


   private:
      /// Get a file, using no locks.
      aFOB innerGetFile( const MC2String& path, bool forceReRead );


      typedef map< MC2String, aFOB > FOBMap;


      /// The file and their FileOBjects
      FOBMap m_files;


      /// The mutex protecting m_files.
      ISABMutex m_mutex;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline aFOB
FOBC::getFile( const MC2String& path, bool forceReRead ) {
   m_mutex.lock();
   aFOB r = innerGetFile( path, forceReRead );
   m_mutex.unlock();
   return r;
}


#endif // FOBC_H

