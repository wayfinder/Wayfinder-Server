/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2GTK_SLOTHANDLER_H
#define MC2GTK_SLOTHANDLER_H

#include <list>
#include <gtk/gtk.h>

#include "Slot.h"

namespace MC2Gtk {

/**
 * Holds Slots and deletes them when it goes out of scope.
 *
 * Example usage with button which will call Test::test()
 * when the button is pressed:
 * \code
 * class Test {
 * public:
 *    void test() { cout << "pressed." << endl }
 * };
 *
 * Test theApp;
 * SlotHandler handler;
 * GtkWidget *somewidget = ...initialize button;
 *
 * handler.connect( G_OBJECT( somewidget ), 
 *                  "clicked", Slot( theApp, &Test::test ) );
 *
 * \endcode
 *
 */
class SlotHandler {
public:   
   SlotHandler() {}
   ~SlotHandler() {      
      while ( !m_slots.empty() ) {
         delete m_slots.back();
         m_slots.pop_back();
      }
   }

   /**
    * Adds slot and returns pointer to slot....
    */
   SlotBase *add( SlotBase *slot ) { m_slots.push_back(slot); return slot; }

   /** Connects a slot to a signal from an object
    * @param obj
    * @param signal_name
    * @param slot
    */
   void connect( GObject *obj, const char *signal_name,
                SlotBase *slot ) {
      g_signal_connect( obj, signal_name,
                        G_CALLBACK(SlotBase::callback),
                        add( slot ) );
   }
private:
   /// make sure we do not try to copy 
   SlotHandler &operator = (const SlotHandler &);
   /// make sure we do not try to copy
   SlotHandler(const SlotHandler &);

   std::list< SlotBase * > m_slots;
};

}

#endif 

