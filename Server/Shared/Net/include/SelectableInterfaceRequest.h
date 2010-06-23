/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SELECTABLEINTERFACEREQUEST_H
#define SELECTABLEINTERFACEREQUEST_H

#include "config.h"
#include "InterfaceRequest.h"

class Selectable;


/**
 * Abstract super class for requests comming from outside mc2 that 
 * uses selectables.
 *
 */
class SelectableInterfaceRequest : public InterfaceRequest {
   public:
      /**
       * Creates a new InterfaceRequest.
       */
      SelectableInterfaceRequest();


      /**
       * Destructor.
       */
      virtual ~SelectableInterfaceRequest();


      /**
       * The irequest should try to get done as soon as possible.
       * This means that reuse of IO for another read of user request
       * is not wanted.
       * Default is to do nothing.
       */
      virtual void terminate();


      /**
       * The selectable.
       */
      virtual Selectable* getSelectable() = 0;


      /**
       * If read is wanted.
       */
      virtual bool wantRead() const = 0;


      /**
       * If write is wanted.
       */
      virtual bool wantWrite() const = 0;


      /**
       * Read and or write is ready make the IO.
       * May only call read/write once and only if 
       * readyRead/readyWrite is true.
       */
      virtual void handleIO( bool readyRead, bool readyWrite ) = 0;


      /**
       * Get the current timeout, in ms.
       */
      uint32 getTimeout() const;


      /**
       * Get the current timeout for the whole process, in s.
       * This should be much longer than the ordinary timeout.
       * Only checked after the ordinary timeout or read/write ready.
       */
      uint32 getTotalTimeout() const;


      /**
       * Get the time when the IO started. In s.
       */
      uint32 getIOStartTime() const;


      /**
       * Sets the time when the IO starts. In s.
       */
      void setIOStartTime( uint32 time );


      /**
       * There was a timeout, change state.
       */
      virtual void timedout() = 0;


      /**
       * Get the used IO time in ms.
       */
      uint32 getUsedIOTime() const;


      /**
       * Set the used time, in ms.
       */
      void setUsedIOTime( uint32 time );


      /**
       * Get the totally used IO time in ms.
       */
      uint32 getTotalUsedIOTime() const;

      /**
       * Set the totally used IO time in ms.
       */
      void setTotalUsedIOTime( uint32 time );


      /**
       * The server is overloaded make mimimal reply with that status.
       */
      virtual void handleOverloaded( int overLoad ) = 0;


      /**
       * If the request idle right now. Waiting for client to reuse
       * connection.
       * Default false.
       */
      virtual bool isIdle() const;


   protected: 
      /// The timeout in ms.
      uint32 m_timeout;


      /// The total timeout in s.
      uint32 m_totalTimeout;


   private:
      /// Used IO time but not yet timeout, in ms.
      uint32 m_usedIOTime;


      /// The total used IO time, in ms.
      uint32 m_totalUsedIOTime;


      /// The time when to IO started, in s.
      uint32 m_ioStartTime;
};


// =======================================================================
//                                     Implementation of inlined methods =


inline uint32 
SelectableInterfaceRequest::getTimeout() const {
   return m_timeout;
}


inline uint32 
SelectableInterfaceRequest::getTotalTimeout() const {
   return m_totalTimeout;
}


inline uint32 
SelectableInterfaceRequest::getIOStartTime() const {
   return m_ioStartTime;
}


inline void 
SelectableInterfaceRequest::setIOStartTime( uint32 time ) {
   m_ioStartTime = time;
}


inline uint32 
SelectableInterfaceRequest::getUsedIOTime() const {
   return m_usedIOTime;
}


inline void 
SelectableInterfaceRequest::setUsedIOTime( uint32 time ) {
   m_usedIOTime = time;
}


inline uint32 
SelectableInterfaceRequest::getTotalUsedIOTime() const {
   return m_totalUsedIOTime;
}


inline void 
SelectableInterfaceRequest::setTotalUsedIOTime( uint32 time ) {
   m_totalUsedIOTime = time;
}


/**
 * Class for comparing SelectableInterfaceRequest's timeouts in 
 * Strict Weak Ordering.
 *
 */
class SelectableInterfaceRequestCmpLess {
   public:
      bool operator()( const SelectableInterfaceRequest* a,
                       const SelectableInterfaceRequest* b ) const
      {
         return (a->getTimeout() - a->getUsedIOTime()) <
            (b->getTimeout() - b->getUsedIOTime());
      }


      bool operator()( const SelectableInterfaceRequest& a,
                       const SelectableInterfaceRequest& b )
      {
         return (a.getTimeout() - a.getUsedIOTime()) <
            (b.getTimeout() - b.getUsedIOTime());
      }
};


#endif // SELECTABLEINTERFACEREQUEST_H

