/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MONITORSET_H
#define MONITORSET_H

#include <set>

#include "ISABThread.h"
/** 
 * A monitor set 
 */
template <typename Type>
class MonitorSet {
public:
   MonitorSet() { }

   bool checkAndLock( const Type& t ) {
      ISABSync sync( m_monitor );
      return m_monitorSet.insert( t ).second;
   }

   void wait( const Type& t ) {
      ISABSync sync( m_monitor );
      while ( m_monitorSet.find( t ) != m_monitorSet.end() ) {
         m_monitor.wait();
      }
   }

   void unlock( const Type& t) {
      ISABSync sync( m_monitor );
      m_monitorSet.erase( t );
      m_monitor.notifyAll();
   } 

private:

   std::set<Type> m_monitorSet;
   ISABMonitor  m_monitor;
};

/**
 * Helper class for unlocking type.
 * Unlocks type when it goes out of scope
 */
template <typename Type>
class LockHelper {
public:
   LockHelper( MonitorSet<Type>& monSet,
               const Type &t):
      m_monSet( monSet ),
      m_type( t ),
      m_locked( false ) {

      while ( ! monSet.checkAndLock( t ) ) {
         monSet.wait( t );         
      }

      m_locked = true;
   }

   ~LockHelper() {
      if ( m_locked ) {
         m_monSet.unlock( m_type );
      }
   }

private:
   MonitorSet<Type> &m_monSet;
   const Type &m_type;
   bool m_locked;
};


#endif // MONITORSET_H
