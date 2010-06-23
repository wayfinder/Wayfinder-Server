/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __LEADERSTATUS_H__
#define __LEADERSTATUS_H__

#include <memory>

class ISABMutex;
/** This class encapsulates the leader/available-status of a module.
 * 
 * The purpose of having this status in its own class rather than as
 * a simple member of Reader is twofold:
 * 
 * 1. Someone might need to know if the module is leader but shouldn't
 *    be tightly coupled to the Reader class.
 * 2. Since knowing the status can be helpful in other threads than the
 *    Reader thread, it must be protected by a mutex, this is done here.
 * 
 */
class LeaderStatus {
public:
   /// The status is false (not leader) at first.
   LeaderStatus();
   
   /// Is the module leader?
   bool isLeader() const;
   
   /// Changes status to leader.
   void becomeLeader();
   
   /// Changes status to available.
   void becomeAvailable();
   
private:
   /// Used to synchronize access to this class.
   std::auto_ptr<ISABMutex> m_mutex;
   
   /// The actual status.
   bool m_leader;
};

#endif //__LEADERSTATUS_H__
