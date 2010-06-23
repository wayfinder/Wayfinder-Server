/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MC2GTK_SLOT_H
#define MC2GTK_SLOT_H

namespace MC2Gtk {

/**
 * Base class for slots
 */
class SlotBase {
public:
   virtual ~SlotBase() { }
   virtual void operator () () = 0;
   static void callback(void *not_used, SlotBase *slot) {
      (*slot)();
   }
};

/**
 * Used for member callbacks in classes
 * Takes one instance to object and member function pointer
 * ( Command pattern )
 */
template <typename Receiver, typename ReturnType=void>
class SlotT: public SlotBase {
public:
   typedef ReturnType (Receiver::* Action)();
   SlotT(Receiver &r, Action a):m_receiver( r ), m_action( a ) {}
   void operator () () {
      (m_receiver.*m_action)();
   }
private:
   Receiver &m_receiver;
   Action m_action;
};

/**
 * Same as SlotT but it also takes one argument
 * and passes it to member functions
 */
template < typename Receiver, typename Argument, 
          typename ReturnType=void >
class SlotT1: public SlotBase {
public:
   typedef ReturnType (Receiver::* Action)( Argument a );
   SlotT1( Receiver &r, Action a, Argument arg ):
      m_receiver( r ), 
      m_action( a ),
      m_argument( arg ) 
   {}

   void operator () () {
      (m_receiver.*m_action)( m_argument );
   }
private:
   Receiver &m_receiver;
   Action m_action;
   Argument m_argument;
};

/**
 * Same as SlotT1 but with two arguments to member function
 */
template < typename Receiver, typename Argument1, 
           typename Argument2, typename ReturnType=void >
class SlotT2: public SlotBase {
public:
   typedef ReturnType (Receiver::* Action)( Argument1 a, Argument2 a2 );
   SlotT2( Receiver &r, Action a, Argument1 arg, Argument2 arg2 ):
      m_receiver( r ), 
      m_action( a ),
      m_argument1( arg ),
      m_argument2( arg2 )
   {}

   void operator () () {
      (m_receiver.*m_action)( m_argument1, m_argument2 );
   }
private:
   Receiver &m_receiver;
   Action m_action;
   Argument1 m_argument1;
   Argument2 m_argument2;
};

/**
 * Helper function, hides SlotT
 */
template < typename Receiver, typename Action >
inline SlotBase *Slot( Receiver& recv, Action a ) {
   return new SlotT< Receiver >( recv, a );
}
/**
 * Helper function, hides SlotT1
 */
template < typename Receiver, typename Action, typename Argument >
inline SlotBase *Slot( Receiver& recv, Action a, Argument arg ) {
   return new SlotT1< Receiver, Argument >( recv, a, arg );
}

/**
 * Helper function, hides SlotT2
 */
template <typename Receiver,typename Action, typename Argument1,typename Argument2>
inline SlotBase *Slot( Receiver& recv, Action a, Argument1 arg1, Argument2 arg2 ) {
   return new SlotT2< Receiver, Argument1, Argument2 >( recv, a, arg1, arg2 );
}

}

#endif // SLOT_H
