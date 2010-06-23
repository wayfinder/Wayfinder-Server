/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SUPERVISOR_DISPLAY_H
#define SUPERVISOR_DISPLAY_H

#include "config.h"

#include "MC2String.h"

#include "SupervisorStorage.h"
#include "Packet.h"
#include "STLStringUtility.h"

#include <iostream>

class SupervisorReceiver;

/**
 *   (Half) abstract class to display the contents of SupervisorStorage.
 *   Inherit from this class when making a new kind of display.
 *   You should use SupervisorStorage::addObserver to receive updates
 *   when something is happening.
 *   @see SupervisorStorage
 */
class SupervisorDisplay : public VectorElement {
  public:
   SupervisorDisplay( SupervisorStorage* s, SupervisorReceiver* r );
   /**
    *   Virtual function to be called from SupervisorStorage notify().
    *   Perhaps more detailed updating types should be added too.
    */
   virtual void update() = 0;

   /**
    * Set alert queue size.
    */
   void setAlertQueueSize( uint32 size ) { m_alertQueueSize = size; }

   /**
    * Set show queue size.
    */
   void setShowQueueSize( uint32 size ) { m_showQueue = size; }


   void setCollate( bool collate ) { m_collate = collate; }

   void setStabilize( bool s ) { m_stabilize = s; }

  protected:
   SupervisorStorage* storage;  
   SupervisorReceiver* m_receiver;

   /// The alertQueue size.
   uint32 m_alertQueueSize;


   /// The showQueue size.
   uint32 m_showQueue;


   /// If alert is on. Starts on is turned off/on with 'w'-key
   bool m_alertOn;


   /// If collate/collapse is on.
   bool m_collate;


   /// If to average numbers for collated infos.
   bool m_average;


   typedef set< MC2String, strNoCaseCompareLess > CollateSet;
   /// The collate/collapsed strings.
   CollateSet m_collateSet;

   /// Creation time of Display
   time_t m_creationTime;

   /**
    * If to wait at startup until things have stabilized before displaying
    * information.
    */
   bool m_stabilize;
};

/**
 *    Implementation of a SupervisorDisplay to display
 *    data in a text window.
 */
class TextDisplay : public SupervisorDisplay {
  public:
   TextDisplay( SupervisorStorage* s, SupervisorReceiver* r );
   /**
    *    Print the contents of SupervisorStorage on the
    *    text display.
    */
   void update();
  protected:
   /// Old displayed screen to avoid flickering.
   MC2String m_oldDisplay;


   /// Old displayed time
   MC2String m_oldDisplayTime;


   /// Last update time to avoid too much work.
   uint32 m_lastUpdateTime;
};

#endif // SUPERVISOR_DISPLAY_H



