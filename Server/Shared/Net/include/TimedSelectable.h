/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef TIMEDSELECTABLE_H
#define TIMEDSELECTABLE_H

#include "Selectable.h"
#include "TimeoutClock.h"
/**
 * A Selectabled that hold timeout on 
 * read and write.
 * It also contains io handling function.
 */
class TimedSelectable: public Selectable {
public:
   TimedSelectable( uint32 workingTimeout = MAX_UINT32, 
                    uint32 idleTimeout = MAX_UINT32 );
   virtual ~TimedSelectable() { }


   inline void setTimeout( uint32 timeout );
   // @return minimum time left of the two clocks
   inline uint32 getTimeLeft() const;
   /// @return absolute time in milliseconds
   inline uint32 getAbsoluteTime() const;

   /// @return true if the timer timed out.
   inline bool timedOut() const;
   /// reset timer
   inline void resetTimeout();

   void setWorking();
   void setIdle();
   /// @return true if this selector wants to be selected with read
   virtual bool wantRead() const = 0;
   /// @return true if this selector wants to be selected with write
   virtual bool wantWrite() const = 0;
   /// called when this selector is ready to read or write
   virtual void handleIO( bool readyRead, bool readyWrite ) = 0;
   /// called when this selector timed out with one of its timers
   virtual void handleTimeout() = 0;


private:
   TimeoutClock m_timer; //< timer
   const uint32 m_workingTimeout; //< working timeout for selectable
   const uint32 m_idleTimeout; //< idle timeout for selectable
};

inline
bool TimedSelectable::timedOut() const
{
   return m_timer.timedOut();
}

inline
uint32 TimedSelectable::getAbsoluteTime() const 
{
   return m_timer.getAbsoluteTime(); 
}

inline
uint32 TimedSelectable::getTimeLeft() const
{
   return m_timer.getTimeLeft();
}

inline
void
TimedSelectable::resetTimeout()
{
   m_timer.reset();
}

inline
void TimedSelectable::setTimeout( uint32 timeout )
{
   m_timer = TimeoutClock( timeout );
}

#endif // TIMEDSELECTABLE_H
