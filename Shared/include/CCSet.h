/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CCSET_H
#define CCSET_H

#define LINKAGE   0
#define HEAD      1
#define LINK      2

#include "config.h"
#include <stdlib.h>

/**
 *    Common base class for Head and Link. Can (and should) not be
 *    instantiated by anyone else than Head and Link classes.
 * 
 */
class Linkage {
   public:
      /** 
       * Head need to instantiate and control attributes
       */
      friend class Head;
      
      /**
       * Link need to instantiate and control attributes
       */
      friend class Link;

      /**
       *    Virtual destructor to enable dynamic casting and to make
       *    sure that the clear()-method in Head deletes the object
       *    correctly.
       */
      virtual ~Linkage();
      
   private:
      /**
       *    Constructor declared private to ensure no instantiation.
       */
      Linkage();

      /**
       *    Constructor declared private to ensure no instantiation.
       *    The members are initiated with values.
       *    @param   n     The linkage that comes after this one.
       *    @param   p     The linkage that comes before this one.
       *    @param   t     The type of object (LINKAGE; HEAD or LINK).
       *    @param   first True if this is the first elemtent, false 
       *                   otherwise.
       *    @param   last  True if this is the last elemtent, false 
       *                   otherwise.
       */
      Linkage( Linkage  *n,
               Linkage  *p,
               int      t,
               bool     first = false,
               bool     last = false );

      /**
       *    The linkage that comes after this one.
       */
      Linkage* m_next;

      /**
       *    The linkage that comes before this one.
       */
      Linkage* m_prev;

      /**
       *    The type of this linkage. Should have one of LINKAGE, HEAD
       *    or LINK. This should really be implemented in some other 
       *    way...
       */
      int m_type;

      /**
       *    True if this is the first element in the list, false 
       *    otherwise.
       */
      bool imFirst;

      /**
       *    True if this is the last element in the list, false 
       *    otherwise.
       */
      bool imLast;
};


/**
 *    Superclass for CCSet lists. 
 *
 *    @see  Link
 */
class Head : public Linkage {
   public:
      /**
       *    Create an empty Head.
       */
      Head();

      /**
       *    Empties the list before destruction.
       */
      virtual ~Head();

      /**
       *    Get the first element in the list.
       *    @return  The first element of the list.
       */
      inline Linkage *first() const;

      /**
       *    Get the last element in the list.
       *    @return  The last element of the list.
       */
      Linkage *last() const;

      /**
       *    Get the number of elements in this list.
       *    @return  The number of elements currently in list.
       */
      inline int cardinal() const;

      /**
       *    Find out if this list is empty or not.
       *    @return  True if this list is empty, false otherwise.
       */
      inline bool empty() const;

      /**
       *    Clear out the contents of this list. Will delete all the
       *    elements (Links) in the list.
       */
      void clear();

      /** 
       *    Link needs access to the attributes.
       */
      friend class Link;

   private:
      /**
       *    The curretn number of elements in the list. Stored in a
       *    member to make the cardinal()-method fast (O(1) instead of
       *    O(n)).
       */
      int card;

      /**
       *    The first element in the list.
       */
      Linkage *m_first;

      /**
       *    The last element in the list.
       */
      Linkage *m_last;
};


/**
 *    Superclass for CCSet list nodes.
 *
 *    @see  Head
 */
class Link : public Linkage {
   public:
      /**
       * Create an empty link
       */
      Link();

      /**
       * Destructor for clearing notices etc.
       */
      virtual ~Link();
      
      /**
       * @return  the successor of this Link in a Head, NULL if
       *          last or not linked into any Head.
       */
      inline Linkage* suc() const;

      /**
       * @return  the predecessor of this Link in a Head, NULL if
       *          first or not linked into any Head.
       */
      inline Linkage* pred() const;

      /**
       * Take this Link out of the Head in which it's linked.
       */
      void out();

      /**
       * Put this Link at the end of a Head.
       *
       * @param   h  The Head into which this Link is to be put
       */
      void into(Head *h);

      /**
       * Put this Link at the beginning of a Head.
       *
       * @param   h  The Head into which this Link is to be put
       */
      void intoAsFirst(Head *h);

      /**
       * Connect this Link into a Head after a given element.
       *
       * @param   linkage  The Linkage after which this Link is
       *                   to be put.
       */
      void follow(Linkage *linkage);

      /**
       * Connect this Link into a Head in front of a given element.
       * 
       * @param   linkage  The Linkage in front of which this
       *                   Link is to be put.
       */
      void precede(Linkage *linkage);
   private:
      Head *m_h;
};

//
// inlines

// HEAD

inline int 
Head::cardinal() const
{
   return card;
}

inline bool 
Head::empty() const
{
   return (card == 0);
}

inline Linkage* 
Head::first() const
{
   return m_first;
}

// LINK

inline Linkage* 
Link::suc() const
{
   return m_next;
}

inline Linkage* 
Link::pred() const
{
   return m_prev;
}


#endif   // CCSET_H

