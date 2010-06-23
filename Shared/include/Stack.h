/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STACK_H
#define STACK_H

#include "config.h"
#include "Vector.h"

/**
  *   An implementation of a "stack". Uses an Vector to keep track of
  *   the elements.
  *
  */
class Stack {
   public:
      /**
        *   Create a "stack" with default size.
        */
      Stack();

      /**
        *   Create a "stack" with specified initial size.
        */
      Stack(uint32 intialSize);
      
      /**
       *
       */
      virtual ~Stack() {}

      /**
        *   Get the value on top in the "stack".
        *   @return  The top value in the "stack". MAX_UINT32 is returned
        *            if no value to pop.
        */
      inline uint32 pop();

      /**
        *   Add a value to the "stack".
        *   @param   The value to insert into the "stack".
        */
      inline void push(uint32 val);

      /**
       *    Get the value of the top-element in the "stack". Does not
       *    change anything in the "stack"!
       *    @return The value of the element in the top of the "stack".
       *            MAX_UINT32 is returned upon error ("stack" empty).
       */
      inline uint32 top();

      /**
       *    Get the number of elements in this "stack".
       *    @return  The number of elements in the "stack" right now.
       */
      inline uint32 getStackSize() const;

      /**
       *    Get the i:th element from the top.
       *    @return The i:th element from the top.
       */
      inline uint32 getFromTop(uint32 i);
      
      /**
       *    Inspect element i (counted from the bottom) of the "stack". 
       *    Use this to inspect an element, but without removing it.
       *    
       *    Note that this should maybe not be a method of a stack, 
       *    however it is added anyway since it is useful in many 
       *    applications.
       *    
       *    @param   i  Index of the (counted from the bottom of the "stack").
       *    @return  The i:th element from the bottom. 
       */
      inline uint32 getElementAt(uint32 i) const;
      /// reserve space for future use
      inline void reserve( uint32 spaceSize );
      
      /**
       *    Reset the "stack", ie. remove all elements.
       */
      inline void reset();

   private:
      /**
        *   The Vector that is the actual "stack".
        */
      Vector m_stackarray;

};


// =======================================================================
//                                 Implementation of the inlined methods =

inline uint32 
Stack::pop()
{
   uint32 retVal = MAX_UINT32;
   if (m_stackarray.getSize() > 0) {
      retVal = m_stackarray.getLast();
      m_stackarray.removeElementAt(m_stackarray.getSize()-1);
   }
   return (retVal);
}

inline void 
Stack::push(uint32 val)
{
   m_stackarray.addLast(val);
}

inline uint32
Stack::top()
{
   uint32 retVal = MAX_UINT32;
   if (m_stackarray.getSize() > 0) {
      retVal = m_stackarray.getLast();
   }
   return (retVal);
}

inline uint32
Stack::getFromTop(uint32 i)
{
   uint32 retVal = MAX_UINT32;
   if (m_stackarray.getSize() > i) {
      retVal = m_stackarray.getElementAt(getStackSize() - 1 - i);
   }
   return (retVal);
}

inline uint32
Stack::getElementAt(uint32 i) const
{
   return (m_stackarray.getElementAt(i));
}

inline uint32
Stack::getStackSize() const 
{
   return (m_stackarray.getSize());
}

inline void
Stack::reset()
{  
   m_stackarray.reset();
}

inline void
Stack::reserve( uint32 spaceSize ) 
{
   m_stackarray.setAllocSize( spaceSize );
}

#endif

