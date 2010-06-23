/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ISABThread.h"

/**
 *  Class allowing several threads to read-access a resource, but
 *  protecting the resource while a thread writes to the resource.
 *  Threads can Read, Write or Delete with this lock
 *  Readers, blocks Writers and Deleters, but not other Readers.
 *  Writers blocks all other threads.
 *  Deleters removes the access to the lock causing alll new Read/Write/Delete
 *  atempts to fail.
 *
 */
class WriteLock
{
  public:
   /**
    *  Constructs a new WriteLock without any Readers, Writers or Deleters.
    */
   inline WriteLock();

   /**
    *  Destructor.
    */
   ~WriteLock(){ }
   
   /**
    *  Adds a reader to the WriteLock.
    *  @return False if a delete is pending.
    */
   inline bool requestRead();

   /**
    *  Reduces the number of readers in the WriteLock by one.
    *  If none remains all sleepers (writers and deleters) are
    *  notified.
    */
   inline void finishRead();

   /**
    *  waits for all Writers to leave the resource
    *  @return False if a delete is pending.
    */
   inline bool requestWrite();

   /**
    *  Reduces the number of writers in the WriteLock by one.
    *  If none remains all sleepers (writers and deleters) are
    *  notified.
    */
   inline void finishWrite();

   /**
    *  
    *  @return False if a delete was already requested.
    */
   inline bool requestDelete();   

   
  private:
   /// The monitor used by the lock.
   ISABMonitor m_monitor;
   
   /**
    *   The number of threads that are reading from the resource.
    */
   uint32 m_nbrOfReaders;

   
   /**
    *  The number of threads that are writing to the resource.
    */
   uint32 m_nbrOfWriters;

   /// True if a thread is writing to the resource.
   bool m_writing;

   /// True if the resource is about to be deleted.
   bool m_deleting;
   
};

// ========================================================================
//                                  Implementation of the inlined methods =

WriteLock::
WriteLock()
{
   m_nbrOfReaders  = 0;
   m_nbrOfWriters  = 0;
   m_deleting       = false;
}


bool 
WriteLock::requestRead()
{
   ISABSync synchronized(m_monitor);
   if(m_deleting)
      return false;
   m_nbrOfReaders++;
   return true;
}

void
WriteLock::finishRead()
{
   ISABSync synchronized(m_monitor);
   m_nbrOfReaders--;
   if(m_nbrOfReaders == 0)
      m_monitor.notifyAll();
}



bool
WriteLock::requestWrite()
{
   ISABSync synchronized(m_monitor);
   if(m_deleting)
      return false;
   m_nbrOfWriters++;
   while((m_writing) || (m_nbrOfReaders != 0)){
      try {
         m_monitor.wait();
      }
      catch(const JTCInterruptedException &) {
         mc2log << warn <<  "Fifo::dequeue wait interrupted!" << endl;
      }
   }
   m_writing = true;
   return true;
}

void
WriteLock::finishWrite()
{
   ISABSync synchronized(m_monitor);
   m_nbrOfWriters--;
   m_writing = false;
   if(m_nbrOfWriters == 0)
      m_monitor.notifyAll();
}

bool
WriteLock::requestDelete()
{
   ISABSync synchronized(m_monitor);
   if(m_deleting)
      return false;
   m_deleting = true;   
   while((m_nbrOfWriters != 0) || (m_nbrOfReaders != 0)){
      try {
         mc2dbg4 << "Nbr of writers = " << m_nbrOfWriters << endl;
         mc2dbg4 << "Nbr of readers = " << m_nbrOfReaders << endl;
         m_monitor.wait();
      }
      catch(const JTCInterruptedException &) {
         mc2log << warn <<  "Fifo::dequeue wait interrupted!" << endl;
      }
   }
   return true;
}
