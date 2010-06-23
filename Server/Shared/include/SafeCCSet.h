/*
Copyright (c) 1999 - 2010, Vodafone Group Services Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the Vodafone Group Services Ltd nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SAFECCSET_H
#define SAFECCSET_H

#include "CCSet.h"
#include "ISABThread.h"


/**
  *   A Safe superclass of the CCSet class Head
  *
  *   @see     SafeLink
  */
class SafeHead : public Head, public ISABMonitor {
   public:
      /**
        *   @return  The first element of the list.
        */
      Linkage *first() {
         JTCSynchronized synchronized(*this);
         return (Head::first());
      }

      /**
       * @return  the last element of the list
       */
      Linkage *last() {
         JTCSynchronized synchronized(*this);
         return (Head::last());
      }


      /**
       * @return  the number of elements currently in list
       */
      const int cardinal() {
         JTCSynchronized synchronized(*this);
         return (Head::cardinal());
      }


      /**
       * @return  true if this Head is empty, false otherwise
       */
      const bool empty() {
         JTCSynchronized synchronized(*this);
         return (Head::empty());
      }


      /**
       * Clear out the contents of this Head
       */
      void clear() {
         JTCSynchronized synchronized(*this);
         Head::clear();
      }
};


/**
  *   Superclass for the safe varianf of the CCSet list nodes.
  *
  *   @see     Head
  */
class SafeLink : public Link, public ISABMonitor {
   public:
      /**
       * @return  the successor of this Link in a Head, NULL if
       *          last or not linked into any Head.
       */
      Linkage *suc() {
         JTCSynchronized synchronized(*this);
         return (Link::suc());
      }

      /**
       * @return  the predecessor of this Link in a Head, NULL if
       *          first or not linked into any Head.
       */
      Linkage *pred() {
         JTCSynchronized synchronized(*this);
         return (Link::pred());
      }


      /**
       * Take this Link out of the Head in which it's linked.
       */
      void out() {
         JTCSynchronized synchronized(*this);
         Link::out();
      }


      /**
       * Put this Link at the end of a Head.
       *
       * @param   h  The Head into which this Link is to be put
       */
      void into(Head *h) {
         JTCSynchronized synchronized(*this);
         Link::into(h);
      }


      /**
       * Put this Link at the beginning of a Head.
       *
       * @param   h  The Head into which this Link is to be put
       */
      void intoAsFirst(Head *h) {
         JTCSynchronized synchronized(*this);
         Link::intoAsFirst(h);
      }


      /**
       * Connect this Link into a Head after a given element.
       *
       * @param   linkage  The Linkage after which this Link is
       *                   to be put.
       */
      void follow(Linkage *linkage) {
         JTCSynchronized synchronized(*this);
         Link::follow(linkage);
      }


      /**
       * Connect this Link into a Head in front of a given element.
       * 
       * @param   linkage  The Linkage in front of which this
       *                   Link is to be put.
       */
      void precede(Linkage *linkage) {
         JTCSynchronized synchronized(*this);
         Link::precede(linkage);
      }
};

#endif

